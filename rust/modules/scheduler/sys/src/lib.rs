/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

//! Providing asynchronous task scheduling and epoll handling mechanism.
//!
//! Current implementation of task scheduler allow posted tasks to run concurrently,
//! so synchronization are necessary if they share some data.
//!
//! On creation of scheduler, an epoll instance is also created and event loop is
//! started. We see a (fd, events processing logic) pair as an event handler. When
//! an epoll handler is added, its fd is added to the interest list of the epoll
//! and is waited on for events. When events occured on its fd, scheduler will
//! dispatch events to it.

#![allow(dead_code)]
#![allow(unused_variables)]

/// Module declarations.
mod scheduler;
mod task;

/// Public exports.
pub use scheduler::IEpollHandler;
pub use scheduler::{
    LIBC_EPOLLIN,
    LIBC_EPOLLERR,
    LIBC_EPOLLHUP,
};
pub use task::TaskHandle;

use std::ffi::{ c_char, CString };
use std::sync::Arc;
use std::sync::atomic::{ AtomicUsize, Ordering };
use std::time::Duration;
use fusion_utils_rust::{call_debug_enter, FusionResult };
use hilog_rust::{ hilog, HiLogLabel, LogType };
use scheduler::Scheduler;

const LOG_LABEL: HiLogLabel = HiLogLabel {
    log_type: LogType::LogCore,
    domain: 0xD002220,
    tag: "Handler",
};

/// Front-end of scheduler, providing interface for posting asynchronous task
/// and epoll handling.
pub struct Handler {
    id: usize,
    scheduler: Arc<Scheduler>,
}

impl Handler {
    /// Construct a new instance of `Handler`.
    pub fn new() -> Self
    {
        static ID_RADIX: AtomicUsize = AtomicUsize::new(1);
        let scheduler = Arc::new(Scheduler::new());

        Self {
            id: ID_RADIX.fetch_add(1, Ordering::Relaxed),
            scheduler,
        }
    }

    /// Return the unique identifier of this `Handler`.
    pub fn id(&self) -> usize
    {
        self.id
    }

    /// Schedudle a `synchronous` executing task, and return the result.
    pub fn post_sync_task<F, R>(&self, task: F) -> R
    where
        F: Fn() -> R + Send + 'static,
        R: Send + 'static,
    {
        call_debug_enter!("Handler::post_sync_task");
        ylong_runtime::block_on(async move {
            task()
        })
    }

    /// Scheduling an asynchronous task.
    ///
    /// Calling `TaskHandle::result` to get the result of the task. Calling
    /// `TaskHandle::result` will block current thread until the task finish.
    ///
    /// Calling `TaskHandle::cancel` to cancel the posted task before it finish.
    ///
    /// # Examples
    ///
    /// ```
    /// let handler = Handler::new();
    /// let param: usize = 0xAB1807;
    ///
    /// let mut task_handle = handler.post_async_task(move || {
    ///     hash(param)
    /// }
    /// let ret = task_handle.result().unwrap();
    /// let expected = hash(param);
    /// assert_eq!(ret, expected);
    /// ```
    ///
    pub fn post_async_task<F, R>(&self, task: F) -> TaskHandle<R>
    where
        F: Fn() -> R + Send + 'static,
        R: Send + 'static,
    {
        call_debug_enter!("Handler::post_async_task");
        let handle = ylong_runtime::spawn(async move {
            task()
        });
        TaskHandle::from(handle)
    }

    /// Schedule an asynchronous task that will run after a period of `delay`.
    ///
    /// Calling `TaskHandle::cancel` to cancel the posted task before it finish.
    ///
    pub fn post_delayed_task<F, R>(&self, task: F, delay: Duration) -> TaskHandle<R>
    where
        F: Fn() -> R + Send + 'static,
        R: Send + 'static,
    {
        call_debug_enter!("Handler::post_delayed_task");
        let handle = ylong_runtime::spawn(async move {
            ylong_runtime::time::sleep(delay).await;
            task()
        });
        TaskHandle::from(handle)
    }

    /// Schedule an asynchronous task that will run repeatedly with set interval
    /// after a period of time.
    ///
    /// The posted task will start to run after a period of `delay` if `delay` is not None.
    /// It will repeat for `repeat` times with `interval` between each running. If `repeat`
    /// is None, the posted task will repeat forever.
    ///
    /// Calling `TaskHandle::cancel` to cancel the posted task before it finish.
    ///
    pub fn post_perioric_task<F>(&self, task: F, delay: Option<Duration>, interval: Duration,
                                 repeat: Option<usize>) -> TaskHandle<()>
    where
        F: Fn() + Send + 'static
    {
        call_debug_enter!("Handler::post_perioric_task");
        let handle = ylong_runtime::spawn(async move {
            if let Some(d) = delay {
                ylong_runtime::time::sleep(d).await;
            }
            ylong_runtime::time::periodic_schedule(task, repeat, interval).await;
        });
        TaskHandle::from(handle)
    }

    /// Schedule an asynchronous task that may block. That is, it may take a huge time to
    /// finish, or may block for resources.
    ///
    /// Calling `TaskHandle::cancel` to cancel the posted task before it finish.
    ///
    pub fn post_blocking_task<F, R>(&self, task: F) -> TaskHandle<R>
    where
        F: Fn() -> R + Send + 'static,
        R: Send + 'static,
    {
        call_debug_enter!("Handler::post_delayed_task");
        let handle = ylong_runtime::spawn_blocking(task);
        TaskHandle::from(handle)
    }

    /// Add an epoll handler to epoll event loop.
    ///
    /// Note that we call a (fd, events processing logic) pair an event handler.
    ///
    /// # Examples
    ///
    /// ```
    /// struct EpollHandler {
    /// //  data members.
    /// }
    ///
    /// impl IEpollHandler for EpollHandler {
    ///     fn fd(&self) -> RawFd {
    ///         // Return fd of this epoll handler.
    ///     }
    ///
    ///     fn dispatch(&self, events: u32) {
    ///         // Process events.
    ///     }
    /// }
    ///
    /// let handler: Arc<Handler> = Arc::default();
    /// let epoll_handler = Arc::new(EpollHandler::new());
    /// handler.add_epoll_handler(epoll_handler)
    /// ```
    pub fn add_epoll_handler(&self, handler: Arc<dyn IEpollHandler>)
        -> FusionResult<Arc<dyn IEpollHandler>>
    {
        call_debug_enter!("Handler::add_epoll_handler");
        self.scheduler.add_epoll_handler(handler)
    }

    /// Remove an epoll handler from epoll event loop.
    pub fn remove_epoll_handler(&self, handler: Arc<dyn IEpollHandler>)
        -> FusionResult<Arc<dyn IEpollHandler>>
    {
        call_debug_enter!("Handler::remove_epoll_handler");
        self.scheduler.remove_epoll_handler(handler)
    }
}

impl Default for Handler {
    fn default() -> Self
    {
        Self::new()
    }
}
