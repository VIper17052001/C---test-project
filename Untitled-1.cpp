#include <iostream>
#include <unistd.h>
#include <cstring>
#include <sys/wait.h>

const int BUFFER_SIZE = 1024;

void child_process(int read_fd, int write_fd) {
    char buffer[BUFFER_SIZE];
    
    while (true) {
        // Зчитуємо дані з каналу
        ssize_t bytesRead = read(read_fd, buffer, BUFFER_SIZE);
        if (bytesRead <= 0) {
            break;
        }
        
        // Перетворюємо текст на верхній регістр
        for (int i = 0; i < bytesRead; ++i) {
            buffer[i] = toupper(buffer[i]);
        }
        
        // Записуємо перетворений текст назад до батьківського процесу
        write(write_fd, buffer, bytesRead);
    }
}

void parent_process(int child_read_fd, int child_write_fd) {
    char buffer[BUFFER_SIZE];
    
    while (true) {
        // Зчитуємо дані зі стандартного введення
        std::cout << "Enter text: ";
        std::cin.getline(buffer, BUFFER_SIZE);
        
        // Перевіряємо на команду завершення
        if (std::strcmp(buffer, "exit") == 0) {
            break;
        }
        
        // Надсилаємо текст дочірньому процесу
        write(child_write_fd, buffer, std::strlen(buffer) + 1);
        
        // Зчитуємо перетворений текст від дочірнього процесу
        ssize_t bytesRead = read(child_read_fd, buffer, BUFFER_SIZE);
        
        // Виводимо перетворений текст на стандартний вивід
        if (bytesRead > 0) {
            std::cout << "Upper case: " << buffer << std::endl;
        }
    }
}

int main() {
    int parent_to_child[2];
    int child_to_parent[2];
    
    // Створюємо два канали
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
        // Дочірній процес
        close(parent_to_child[1]); // Закриваємо непотрібні кінці каналів
        close(child_to_parent[0]);
        child_process(parent_to_child[0], child_to_parent[1]);
        close(parent_to_child[0]);
        close(child_to_parent[1]);
    } else {
        // Батьківський процес
        close(parent_to_child[0]); // Закриваємо непотрібні кінці каналів
        close(child_to_parent[1]);
        parent_process(child_to_parent[0], parent_to_child[1]);
        close(parent_to_child[1]);
        close(child_to_parent[0]);
        
        // Чекаємо завершення дочірнього процесу
        wait(NULL);
    }
    
    return 0;
}