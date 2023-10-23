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

//! Tests of asynchronous scheduling.

use std::ffi::{ c_void, c_char, c_int, CString };
use std::io::Error;
use std::os::fd::RawFd;
use std::sync::{ Arc, Condvar, Mutex };
use std::sync::atomic::{ AtomicI32, Ordering };
use std::time::Duration;

use hilog_rust::{ debug, info, error, hilog, HiLogLabel, LogType };

use fusion_scheduler_rust::{ Handler, IEpollHandler, LIBC_EPOLLIN };
use fusion_utils_rust::call_debug_enter;

const LOG_LABEL: HiLogLabel = HiLogLabel {
    log_type: LogType::LogCore,
    domain: 0xD002220,
    tag: "FusionSchedulerTest",
};

struct EpollHandlerImpl {
    fds: [RawFd; 2],
    data: i32,
}

impl EpollHandlerImpl {
    fn signal(&self, data: i32)
    {
        error!(LOG_LABEL, "EpollHandlerImpl::signal once");
        let ret = unsafe {
            libc::write(self.fds[1], std::ptr::addr_of!(data) as *const c_void, std::mem::size_of_val(&data))
        };
        if ret == -1 {
            error!(LOG_LABEL, "libc::write fail");
        }
    }

    fn fd(&self) -> RawFd
    {
        self.fds[0]
    }

    fn dispatch(&mut self, events: u32)
    {
        call_debug_enter!("EpollHandlerImpl::dispatch");
        if (events & LIBC_EPOLLIN) == LIBC_EPOLLIN {
            let data: i32 = 0;

            let ret = unsafe {
                libc::read(self.fds[0], std::ptr::addr_of!(data) as *mut c_void, std::mem::size_of_val(&data))
            };
            if ret == -1 {
                error!(LOG_LABEL, "libc::read fail");
            }
            info!(LOG_LABEL, "EpollHandlerImpl::dispatch({}), data:{}", @public(self.fds[0]), @public(data));
            self.data = data;
        }
    }

    fn data(&self) -> i32
    {
        self.data
    }
}

impl Drop for EpollHandlerImpl {
    fn drop(&mut self)
    {
        for fd in &mut self.fds {
            if *fd != -1 {
                unsafe { libc::close(*fd) };
                *fd = -1;
            }
        }
    }
}

struct EpollHandler {
    inner: Mutex<EpollHandlerImpl>,
    var: Condvar,
}

impl EpollHandler {
    fn new() -> Self
    {
        let mut fds: [c_int; 2] = [-1; 2];

        let ret = unsafe { libc::pipe2(fds.as_mut_ptr(), libc::O_CLOEXEC | libc::O_NONBLOCK) };
        if ret != 0 {
            error!(LOG_LABEL, "In EpollHandler::new, libc::pipe2 fail:{:?}", @public(Error::last_os_error()));
        }
        debug!(LOG_LABEL, "In EpollHandler::new, fds:({},{})", @public(fds[0]), @public(fds[1]));
        Self {
            inner: Mutex::new(EpollHandlerImpl {
                fds,
                data: -1,
            }),
            var: Condvar::new(),
        }
    }

    fn signal(&self, data: i32)
    {
        let guard = self.inner.lock().unwrap();
        guard.signal(data);
    }

    fn data(&self) -> i32
    {
        let guard = self.inner.lock().unwrap();
        guard.data()
    }

    fn wait(&self, dur: Duration) -> bool
    {
        call_debug_enter!("EpollHandler::wait");
        let guard = self.inner.lock().unwrap();
        let (_, ret) = self.var.wait_timeout(guard, dur).unwrap();
        if ret.timed_out() {
            info!(LOG_LABEL, "In EpollHandler::wait, timeout");
            false
        } else {
            true
        }
    }
}

impl IEpollHandler for EpollHandler {
    fn fd(&self) -> RawFd
    {
        let guard = self.inner.lock().unwrap();
        guard.fd()
    }

    fn dispatch(&self, events: u32)
    {
        call_debug_enter!("EpollHandler::dispatch");
        let mut guard = self.inner.lock().unwrap();
        guard.dispatch(events);
        self.var.notify_one();
    }
}

#[test]
fn test_add_epoll_handler()
{
    let handler: Arc<Handler> = Arc::default();
    let epoll = Arc::new(EpollHandler::new());
    assert!(handler.add_epoll_handler(epoll.clone()).is_ok());

    let data: i32 = 13574;
    epoll.signal(data);
    assert!(epoll.wait(Duration::from_millis(100)));
    info!(LOG_LABEL, "In test_add_epoll_handler, data:{}", @public(epoll.data()));
    assert_eq!(epoll.data(), data);
    assert!(handler.remove_epoll_handler(epoll).is_ok());
}

fn hash(param: usize) -> usize
{
    const HASHER: usize = 0xAAAAAAAA;
    HASHER ^ param
}

#[test]
fn test_post_sync_task()
{
    let handler: Arc<Handler> = Arc::default();
    let param: usize = 0xAB1807;

    let ret = handler.post_sync_task(move || {
        hash(param)
    });
    let expected = hash(param);
    assert_eq!(ret, expected);
}

#[test]
fn test_post_async_task()
{
    let handler: Arc<Handler> = Arc::default();
    let param: usize = 0xAB1807;

    let mut task_handle = handler.post_async_task(move || {
        hash(param)
    });
    let ret = task_handle.result().unwrap();
    let expected = hash(param);
    assert_eq!(ret, expected);
}

#[test]
fn test_post_perioric_task()
{
    let handler: Arc<Handler> = Arc::default();
    let epoll = Arc::new(EpollHandler::new());
    let cloned_epoll = epoll.clone();
    assert!(handler.add_epoll_handler(epoll.clone()).is_ok());

    let _ = handler.post_perioric_task(move || {
        static ID_RADIX: AtomicI32 = AtomicI32::new(1);
        cloned_epoll.signal(ID_RADIX.fetch_add(1, Ordering::Relaxed));
    }, None, Duration::from_millis(100), Some(10));

    std::thread::sleep(Duration::from_secs(1));
    info!(LOG_LABEL, "In test_post_perioric_task, data:{}", @public(epoll.data()));
    assert!(epoll.data() >= 10);
    assert!(handler.remove_epoll_handler(epoll).is_ok());
}

#[test]
fn test_post_delayed_task()
{
    let handler: Arc<Handler> = Arc::default();
    let epoll = Arc::new(EpollHandler::new());
    assert!(handler.add_epoll_handler(epoll.clone()).is_ok());
    let data: i32 = 13547;
    let cloned_epoll = epoll.clone();

    let _ = handler.post_delayed_task(move || {
        cloned_epoll.signal(data);
    }, Duration::from_millis(10));

    assert!(epoll.wait(Duration::from_millis(100)));
    info!(LOG_LABEL, "In test_post_delayed_task, data:{}", @public(epoll.data()));
    assert_eq!(epoll.data(), data);
    assert!(handler.remove_epoll_handler(epoll).is_ok());
}

#[test]
fn test_post_blocking_task()
{
    let handler: Arc<Handler> = Arc::default();
    let param: usize = 0xAB1807;

    let mut task_handle = handler.post_blocking_task(move || {
        std::thread::sleep(Duration::from_millis(100));
        hash(param)
    });
    let ret = task_handle.result().unwrap();
    let expected = hash(param);
    assert_eq!(ret, expected);
}
