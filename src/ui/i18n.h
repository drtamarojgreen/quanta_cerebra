#ifndef I18N_H
#define I18N_H
#include <string>
#include <map>

class I18n {
    static inline std::string current_lang = "en";
    static inline std::map<std::string, std::map<std::string, std::string>> dict = {
        {"en", {{"pause", "P"}, {"faster", "F"}, {"slower", "S"}}},
        {"es", {{"pause", "Pausa"}, {"faster", "Mas rapido"}, {"slower", "Mas lento"}}}
    };
public:
    static void setLanguage(const std::string& lang) { current_lang = lang; }
    static std::string get(const std::string& key) {
        if(dict.count(current_lang) && dict[current_lang].count(key)) return dict[current_lang][key];
        return key;
    }
};

#endif
