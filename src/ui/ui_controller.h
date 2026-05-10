#pragma once

#include <vector>
#include <string>
#include <map>
#include <deque>
#include <iostream>
#include <vector>
#include <string>
#include <limits> // For numeric_limits
#include <algorithm> // For std::max, std::min
#include <cstdio> // For getchar

// Forward declare AppConfig and SimulationState if they are defined elsewhere
// For now, assuming they exist and are accessible.
struct AppConfig; 
enum class SimulationState;

// Enum for menu states
enum class MenuState {
    STARTUP_MENU,
    MAIN_MENU,
    LOAD_FILE,
    RUN_TESTS,
    CONFIG_VIEW,
    RUN_SIMULATION,
    QUIT
};

// Structure for a menu item
struct MenuItem {
    char shortcut;
    std::string description;
    MenuState next_state;
};

class MenuManager {
public:
    MenuManager(/* AppConfig& config */);
    ~MenuManager();

    void run();

    MenuState getCurrentState() const { return current_state; }

private:
    void renderMenu();
    void handleInput();
    void transitionTo(MenuState newState);

    void setupStartupMenu();
    void setupMainMenu();
    void setupLoadFileMenu();
    void setupRunTestsMenu();
    void setupConfigViewMenu();
    // ... other menu setup methods ...

    MenuState current_state;
    std::vector<MenuItem> current_menu_items;
    // AppConfig& app_config; 
};
