#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <fstream>
#include <sstream>
#include <thread>
#include <curses.h>
#include <boost/process.hpp>
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;
namespace bp = boost::process;

class CodeSandbox {
private:
    WINDOW* editor_win;
    WINDOW* output_win;
    WINDOW* status_win;
    std::vector<std::string> code_buffer;
    std::string temp_dir;
    int current_line;
    int cursor_x;
    bool running;

    void init_windows() {
        initscr();
        start_color();
        cbreak();
        noecho();
        keypad(stdscr, TRUE);

        // Color pairs
        init_pair(1, COLOR_GREEN, COLOR_BLACK);  // Status
        init_pair(2, COLOR_CYAN, COLOR_BLACK);   // Editor
        init_pair(3, COLOR_YELLOW, COLOR_BLACK); // Output

        int max_y, max_x;
        getmaxyx(stdscr, max_y, max_x);

        // Create windows
        editor_win = newwin(max_y - 8, max_x / 2, 0, 0);
        output_win = newwin(max_y - 8, max_x / 2, 0, max_x / 2);
        status_win = newwin(8, max_x, max_y - 8, 0);

        // Enable scrolling for output window
        scrollok(output_win, TRUE);

        // Draw borders
        box(editor_win, 0, 0);
        box(output_win, 0, 0);
        box(status_win, 0, 0);

        // Set colors
        wattron(editor_win, COLOR_PAIR(2));
        wattron(output_win, COLOR_PAIR(3));
        wattron(status_win, COLOR_PAIR(1));

        // Titles
        mvwprintw(editor_win, 0, 2, "[ Code Editor ]");
        mvwprintw(output_win, 0, 2, "[ Output ]");
        mvwprintw(status_win, 0, 2, "[ Status ]");

        refresh();
        wrefresh(editor_win);
        wrefresh(output_win);
        wrefresh(status_win);
    }

    void cleanup_windows() {
        delwin(editor_win);
        delwin(output_win);
        delwin(status_win);
        endwin();
    }

    void save_code() {
        fs::path temp_file = fs::path(temp_dir) / "temp.cpp";
        std::ofstream file(temp_file.string());
        
        // Add includes and main function if not present
        bool has_main = false;
        bool has_includes = false;
        
        for (const auto& line : code_buffer) {
            if (line.find("main") != std::string::npos) has_main = true;
            if (line.find("#include") != std::string::npos) has_includes = true;
            file << line << "\n";
        }
        
        if (!has_includes) {
            file.seekp(0);
            file << "#include <iostream>\n#include <string>\n#include <vector>\n";
            for (const auto& line : code_buffer) {
                file << line << "\n";
            }
        }
        
        if (!has_main) {
            file << "\nint main() {\n";
            for (const auto& line : code_buffer) {
                file << "    " << line << "\n";
            }
            file << "    return 0;\n}\n";
        }
        
        file.close();
    }

    void compile_and_run() {
        save_code();
        fs::path temp_file = fs::path(temp_dir) / "temp.cpp";
        fs::path output_file = fs::path(temp_dir) / "program";

        // Clear output window
        werase(output_win);
        box(output_win, 0, 0);
        mvwprintw(output_win, 0, 2, "[ Output ]");

        // Compile
        std::string compile_cmd = "g++ -std=c++17 " + temp_file.string() + " -o " + output_file.string();
        bp::ipstream compile_output;
        bp::system(compile_cmd, bp::std_err > compile_output);

        std::string line;
        int output_y = 1;
        while (std::getline(compile_output, line)) {
            mvwprintw(output_win, output_y++, 1, line.c_str());
        }

        // Run if compilation successful
        if (fs::exists(output_file)) {
            mvwprintw(output_win, output_y++, 1, "=== Program Output ===");
            bp::ipstream program_output;
            bp::system(output_file.string(), bp::std_out > program_output);

            while (std::getline(program_output, line)) {
                mvwprintw(output_win, output_y++, 1, line.c_str());
            }
        }

        wrefresh(output_win);
    }

    void handle_input() {
        int ch = wgetch(editor_win);
        switch (ch) {
            case KEY_UP:
                if (current_line > 0) current_line--;
                break;
            case KEY_DOWN:
                if (current_line < static_cast<int>(code_buffer.size())) current_line++;
                break;
            case KEY_LEFT:
                if (cursor_x > 0) cursor_x--;
                break;
            case KEY_RIGHT:
                if (cursor_x < static_cast<int>(code_buffer[current_line].length())) cursor_x++;
                break;
            case '\n':
                code_buffer.insert(code_buffer.begin() + current_line + 1, "");
                current_line++;
                cursor_x = 0;
                break;
            case KEY_BACKSPACE:
            case 127:
                if (cursor_x > 0) {
                    code_buffer[current_line].erase(cursor_x - 1, 1);
                    cursor_x--;
                } else if (current_line > 0) {
                    cursor_x = code_buffer[current_line - 1].length();
                    code_buffer[current_line - 1] += code_buffer[current_line];
                    code_buffer.erase(code_buffer.begin() + current_line);
                    current_line--;
                }
                break;
            case KEY_F(5):
                compile_and_run();
                break;
            case KEY_F(10):
                running = false;
                break;
            default:
                if (ch >= 32 && ch <= 126) {
                    if (current_line >= static_cast<int>(code_buffer.size())) {
                        code_buffer.push_back("");
                    }
                    code_buffer[current_line].insert(cursor_x, 1, static_cast<char>(ch));
                    cursor_x++;
                }
                break;
        }
    }

    void update_display() {
        werase(editor_win);
        box(editor_win, 0, 0);
        mvwprintw(editor_win, 0, 2, "[ Code Editor ]");

        for (size_t i = 0; i < code_buffer.size(); i++) {
            mvwprintw(editor_win, i + 1, 1, "%s", code_buffer[i].c_str());
        }

        // Update status
        werase(status_win);
        box(status_win, 0, 0);
        mvwprintw(status_win, 0, 2, "[ Status ]");
        mvwprintw(status_win, 1, 1, "F5: Compile & Run | F10: Exit");
        mvwprintw(status_win, 2, 1, "Line: %d, Column: %d", current_line + 1, cursor_x + 1);

        wmove(editor_win, current_line + 1, cursor_x + 1);
        wrefresh(editor_win);
        wrefresh(status_win);
    }

public:
    CodeSandbox() : current_line(0), cursor_x(0), running(true) {
        // Create temporary directory
        temp_dir = fs::temp_directory_path().string() + "/cpp_sandbox_" + 
                   std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
        fs::create_directory(temp_dir);

        init_windows();
        code_buffer.push_back("");
    }

    ~CodeSandbox() {
        cleanup_windows();
        fs::remove_all(temp_dir);
    }

    void run() {
        while (running) {
            update_display();
            handle_input();
        }
    }
};

int main() {
    try {
        CodeSandbox sandbox;
        sandbox.run();
    } catch (const std::exception& e) {
        endwin();
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
