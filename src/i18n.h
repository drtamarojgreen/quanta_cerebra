#include <map>
#include <string>

class I18n {
public:
    static std::string get(const std::string& key, const std::string& lang = "en") {
        static std::map<std::string, std::map<std::string, std::string>> dict = {
            {"en", {{"pause", "Pause"}, {"speed", "Speed"}}},
            {"es", {{"pause", "Pausa"}, {"speed", "Velocidad"}}},
            {"fr", {{"pause", "Pause"}, {"speed", "Vitesse"}}},
            {"de", {{"pause", "Pause"}, {"speed", "Geschwindigkeit"}}}
        };
        if (dict.count(lang) && dict[lang].count(key)) return dict[lang][key];
        return key;
    }
};
