#include <iostream>
#include <unistd.h>
#include <cstring>
#include <sys/wait.h>

void child_process(int read_fd, int write_fd) {
    char ch;
    
    while (true) {
        ssize_t bytesRead = read(read_fd, &ch, 1);
        if (bytesRead <= 0) break;
        
     
        ch = toupper(ch);
        
      
        write(write_fd, &ch, 1);
    }
}

void parent_process(int child_read_fd, int child_write_fd) {
    char ch;
    
    while (true) {
        std::cout << "Enter text: ";
        
       
        while (std::cin.get(ch) && ch != '\n') {
            
            write(child_write_fd, &ch, 1);
            
       
            ssize_t bytesRead = read(child_read_fd, &ch, 1);
            
            
            if (bytesRead > 0) {
                std::cout << ch;
            }
        }
       
        if (std::strncmp(&ch, "exit", 4) == 0) {
            break;
        }
        
        std::cout << std::endl;
    }
}

int main() {
    int parent_to_child[2];
    int child_to_parent[2];
    
    if (pipe(parent_to_child) == -1 || pipe(child_to_parent) == -1) {
        std::cerr << "Failed to create pipes" << std::endl;
        return 1;
    }
    
    pid_t pid = fork();
    
    if (pid == -1) {
        std::cerr << "Failed to fork" << std::endl;
        return 1;
    }
    
    if (pid == 0) {
        close(parent_to_child[1]);
        close(child_to_parent[0]);
        child_process(parent_to_child[0], child_to_parent[1]);
        close(parent_to_child[0]);
        close(child_to_parent[1]);
    } else {
        close(parent_to_child[0]);
        close(child_to_parent[1]);
        parent_process(child_to_parent[0], parent_to_child[1]);
        close(parent_to_child[1]);
        close(child_to_parent[0]);
        
        wait(NULL);
    }
    
    return 0;
}
