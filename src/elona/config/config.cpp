#include "config.hpp"
#include <cassert>
#include <fstream>
#include <functional>
#include <stdexcept>
#include <string>
#include "../../snail/android.hpp"
#include "../../snail/application.hpp"
#include "../../snail/touch_input.hpp"
#include "../../snail/window.hpp"
#include "../../util/fps_counter.hpp"
#include "../../util/range.hpp"
#include "../draw.hpp"
#include "../elona.hpp"
#include "../hcl.hpp"
#include "../variables.hpp"



namespace
{

struct ConfigLoadingError : public std::runtime_error
{
    ConfigLoadingError(const std::string& message)
        : std::runtime_error(message)
    {
    }
};



template <typename Iterator, typename Function>
void for_each_with_index(Iterator first, Iterator last, Function f)
{
    for (size_t index = 0; first != last; ++first, ++index)
    {
        (void)f(index, *first);
    }
}



void write_default_config(const fs::path& filepath)
{
    // { core: {} }
    json5::value::object_type object;
    object.emplace("core", json5::value::object_type{});

    std::ofstream out{filepath.native()};
    json5::stringify_options opts;
    opts.prettify = true;
    opts.sort_by_key = true;
    opts.unquote_key = true;
    opts.insert_trailing_comma = true;
    out << json5::stringify(object, opts) << std::endl;
}



/***
 * Initializes the list of available display modes. To be called after
 * the application has been initialized by calling title().
 */
void inject_display_modes(Config& conf)
{
    const auto display_modes =
        snail::Application::instance().get_display_modes();
    std::string default_display_mode =
        snail::Application::instance().get_default_display_mode();
    std::vector<std::string> display_mode_names;
    std::string current_display_mode = Config::instance().display_mode;

    bool config_display_mode_found = false;
    int index = 0;

    for (const auto pair : display_modes)
    {
        // First pair member contains identifier string, second is SDL
        // display mode struct.
        display_mode_names.emplace_back(pair.first);

        // If this is the display mode currently selected in the
        // config, mark that it's been found.
        if (pair.first == current_display_mode)
        {
            config_display_mode_found = true;
        }
        index++;
    }

    // If the display mode in the config was not found, reconfigure it to
    // the application's default.
    if (!config_display_mode_found || current_display_mode == "")
    {
        current_display_mode = default_display_mode;
    }

    // If the display_mode is still unknown, we're probably in
    // headless mode, so don't try to set any config options (or
    // "invalid enum variant" will be generated).
    if (current_display_mode != "")
    {
        conf.inject_enum(
            "core.screen.display_mode",
            display_mode_names,
            default_display_mode);

        if (Config::instance().display_mode == spec::unknown_enum_variant)
        {
            Config::instance().set(
                "core.screen.display_mode", default_display_mode);
        }
    }
}

/***
 * Initializes the list of save files that can be chosen at startup.
 */
void inject_save_files(Config& conf)
{
    std::vector<std::string> saves;
    saves.push_back("");

    if (fs::exists(filesystem::dirs::save()))
    {
        for (const auto& entry :
             filesystem::glob_dirs(filesystem::dirs::save()))
        {
            std::string folder =
                filepathutil::to_utf8_path(entry.path().filename());
            saves.push_back(folder);
        }
    }

    conf.inject_enum("core.game.default_save", saves, "");
}

/***
 * Initializes the list of languages by adding the names of folders in
 * the locale/ directory.
 *
 * TODO: Support mods which add their own languages.
 */
void inject_languages(Config& conf)
{
    std::vector<std::string> locales;
    bool has_jp = false;
    bool has_en = false;

    for (const auto& entry : filesystem::glob_dirs(filesystem::dirs::locale()))
    {
        std::string identifier =
            filepathutil::to_utf8_path(entry.path().filename());
        locales.push_back(identifier);

        if (identifier == "en")
        {
            has_en = true;
        }
        if (identifier == "jp")
        {
            has_jp = true;
        }
    }

    // Not having English or Japanese loaded will cause weird things
    // to happen, since many parts of the code depend on one or the
    // other being loaded. This can be removed after those parts of
    // the code are refactored.
    if (!has_en || !has_jp)
    {
        throw ConfigLoadingError(
            "Locale for English or Japanese is missing in locale/ folder.");
    }

    conf.inject_enum(
        "core.language.language", locales, spec::unknown_enum_variant);
}



std::map<std::string, snail::android::Orientation> orientations = {
    {"sensor_landscape", snail::android::Orientation::sensor_landscape},
    {"sensor_portait", snail::android::Orientation::sensor_portrait},
    {"sensor", snail::android::Orientation::sensor},
    {"landscape", snail::android::Orientation::landscape},
    {"portrait", snail::android::Orientation::portrait},
    {"reverse_landscape", snail::android::Orientation::reverse_landscape},
    {"reverse_portrait", snail::android::Orientation::reverse_portrait}};



void convert_and_set_requested_orientation(std::string variant)
{
    auto it = orientations.find(variant);
    if (it == orientations.end())
        return;

    snail::android::set_requested_orientation(it->second);
}



void convert_and_set_requested_font_quality(std::string variant)
{
    if (variant == "low")
    {
        snail::Application::instance()
            .get_renderer()
            .disable_blended_text_rendering();
    }
    else
    {
        if (variant != "high")
        {
            // Unknown font quality; fallback to the default value, "high".
            ELONA_WARN("config") << "Unsupported font quality: " << variant;
        }
        snail::Application::instance()
            .get_renderer()
            .enable_blended_text_rendering();
    }
}



void set_touch_quick_action_transparency(int factor)
{
    float amount = (float)factor * 0.05f;
    snail::TouchInput::instance().set_quick_action_transparency(amount);
}



void set_touch_quick_action_size(int factor)
{
    float size = (float)factor * 0.025f;
    snail::TouchInput::instance().set_base_quick_action_size(size);
    snail::TouchInput::instance().initialize_quick_actions();
}

} // namespace



namespace elona
{

void config_query_language()
{
    constexpr snail::Color bg_color{160, 145, 128};
    constexpr snail::Color fg_color{71, 64, 55};

    gsel(0);
    gmode(0);
    int cursor = 0;

    const auto prev_font = Config::instance().font_filename;
    // Because this Japanese font has more glyph than English one does, it can
    // display language names more correctly.
    Config::instance().font_filename =
        "GenShinGothic/GenShinGothic-Monospace-Regular.ttf";
    font(16);

    while (true)
    {
        boxf(0, 0, windoww, windowh, bg_color);
        mes(40, 40, u8"Choose your language and press Enter key.", fg_color);
        mes(40, 60, u8"言語を選びENTERキーを押してください。", fg_color);
        mes(50, 90, u8"Japanese (日本語)", fg_color);
        mes(50, 110, u8"English", fg_color);
        mes(35, cursor == 0 ? 90 : 110, u8">", fg_color);
        redraw();
        await(30);
        if (getkey(snail::Key::down))
        {
            cursor = 1;
        }
        if (getkey(snail::Key::keypad_2))
        {
            cursor = 1;
        }
        if (getkey(snail::Key::up))
        {
            cursor = 0;
        }
        if (getkey(snail::Key::keypad_8))
        {
            cursor = 0;
        }
        if (getkey(snail::Key::enter))
        {
            break;
        }
        if (getkey(snail::Key::keypad_enter))
        {
            break;
        }
        if (getkey(snail::Key::space))
        {
            break;
        }
    }

    std::string locale = spec::unknown_enum_variant;
    if (cursor == 0)
    {
        locale = "jp";
    }
    else
    {
        locale = "en";
    }
    Config::instance().set(u8"core.language.language", locale);

    Config::instance().font_filename = prev_font;
}


#define CONFIG_OPTION(confkey, type, getter) \
    conf.bind_getter("core."s + confkey, [&]() { return (getter); }); \
    conf.bind_setter<type>( \
        "core."s + confkey, [&](auto value) { getter = value; })

#define CONFIG_KEY(confkey, keyname) \
    CONFIG_OPTION((confkey), std::string, keyname)


void load_config()
{
    const fs::path config_file = filesystem::files::profile_local_config();
    auto& conf = Config::instance();

    // TODO do inversions
    // clang-format off
    CONFIG_OPTION("anime.alert_wait"s, json5::integer_type, Config::instance().alert_wait);
    CONFIG_OPTION("anime.anime_wait"s, json5::integer_type, Config::instance().animation_wait);
    CONFIG_OPTION("anime.attack_anime"s, bool, Config::instance().attack_animation);
    CONFIG_OPTION("anime.auto_turn_speed"s, std::string, Config::instance().auto_turn_speed);
    CONFIG_OPTION("anime.general_wait"s, json5::integer_type, Config::instance().general_wait);
    CONFIG_OPTION("anime.screen_refresh"s, json5::integer_type, Config::instance().screen_refresh_wait);
    CONFIG_OPTION("anime.scroll"s, bool, Config::instance().scroll);
    CONFIG_OPTION("anime.scroll_when_run"s, bool, Config::instance().scroll_when_run);
    CONFIG_OPTION("anime.title_effect"s, bool, Config::instance().title_effect);
    CONFIG_OPTION("anime.weather_effect"s, bool, Config::instance().weather_effect);
    CONFIG_OPTION("anime.window_anime"s, bool, Config::instance().window_animation);
    CONFIG_OPTION("balance.restock_interval"s, json5::integer_type, Config::instance().restock_interval);
    CONFIG_OPTION("font.file"s, std::string, Config::instance().font_filename);
    CONFIG_OPTION("font.size_adjustment"s, json5::integer_type, sizefix);
    CONFIG_OPTION("font.vertical_offset"s, json5::integer_type, vfix);
    CONFIG_OPTION("foobar.autopick"s, bool, Config::instance().autopick);
    CONFIG_OPTION("foobar.autosave"s, bool, Config::instance().autosave);
    CONFIG_OPTION("foobar.damage_popup"s, bool, Config::instance().damage_popup);
    CONFIG_OPTION("foobar.hp_bar_position"s, std::string, Config::instance().hp_bar_position);
    CONFIG_OPTION("foobar.leash_icon"s, bool, Config::instance().leash_icon);
    CONFIG_OPTION("foobar.max_damage_popup"s, json5::integer_type, Config::instance().max_damage_popup);
    CONFIG_OPTION("foobar.allow_enhanced_skill_tracking"s, bool, Config::instance().allow_enhanced_skill);
    CONFIG_OPTION("foobar.enhanced_skill_tracking_lowerbound"s, json5::integer_type, Config::instance().enhanced_skill_lowerbound);
    CONFIG_OPTION("foobar.enhanced_skill_tracking_upperbound"s, json5::integer_type, Config::instance().enhanced_skill_upperbound);
    CONFIG_OPTION("foobar.startup_script"s, std::string, Config::instance().startup_script);
    CONFIG_OPTION("foobar.pcc_graphic_scale"s, std::string, Config::instance().pcc_graphic_scale);
    CONFIG_OPTION("foobar.skip_confirm_at_shop"s, bool, Config::instance().skip_confirm_at_shop);
    CONFIG_OPTION("foobar.skip_overcasting_warning"s, bool, Config::instance().skip_overcasting_warning);
    CONFIG_OPTION("game.attack_neutral_npcs"s, bool, Config::instance().attack_neutral_npcs);
    CONFIG_OPTION("game.extra_help"s, bool, Config::instance().extra_help);
    CONFIG_OPTION("game.hide_autoidentify"s, bool, Config::instance().hide_autoidentify);
    CONFIG_OPTION("game.hide_shop_updates"s, bool, Config::instance().hide_shop_updates);
    CONFIG_OPTION("game.story"s, bool, Config::instance().story);
    CONFIG_OPTION("input.attack_wait"s, json5::integer_type, Config::instance().attack_wait);
    CONFIG_OPTION("input.autodisable_numlock"s, bool, Config::instance().autodisable_numlock);
    CONFIG_OPTION("input.key_wait"s, json5::integer_type, Config::instance().key_wait);
    CONFIG_OPTION("input.initial_key_repeat_wait"s, json5::integer_type, Config::instance().initial_key_repeat_wait);
    CONFIG_OPTION("input.key_repeat_wait"s, json5::integer_type, Config::instance().key_repeat_wait);
    CONFIG_OPTION("input.walk_wait"s, json5::integer_type, Config::instance().walk_wait);
    CONFIG_OPTION("input.run_wait"s, json5::integer_type, Config::instance().run_wait);
    CONFIG_OPTION("input.start_run_wait"s, json5::integer_type, Config::instance().start_run_wait);
    CONFIG_OPTION("input.select_wait"s, json5::integer_type, Config::instance().select_wait);
    CONFIG_OPTION("input.select_fast_start_wait"s, json5::integer_type, Config::instance().select_fast_start_wait);
    CONFIG_OPTION("input.select_fast_wait"s, json5::integer_type, Config::instance().select_fast_wait);
    CONFIG_OPTION("message.add_timestamps"s, bool, Config::instance().message_add_timestamps);
    CONFIG_OPTION("message.transparency"s, json5::integer_type, Config::instance().message_transparency);
    CONFIG_OPTION("net.is_enabled"s, bool, Config::instance().net);
    CONFIG_OPTION("anime.always_center"s, bool, Config::instance().always_center);
    CONFIG_OPTION("screen.music"s, bool, Config::instance().music);
    CONFIG_OPTION("screen.sound"s, bool, Config::instance().sound);
    CONFIG_OPTION("screen.heartbeat"s, bool, Config::instance().heartbeat);
    CONFIG_OPTION("screen.high_quality_shadows"s, bool, Config::instance().high_quality_shadow);
    CONFIG_OPTION("screen.object_shadows"s, bool, Config::instance().object_shadow);
    CONFIG_OPTION("screen.skip_random_event_popups"s, bool, Config::instance().skip_random_event_popups);
    // clang-format on

    conf.bind_setter<std::string>(
        "core.screen.orientation", &convert_and_set_requested_orientation);

    conf.bind_setter<bool>(
        "core.foobar.show_fps", [](bool) { lib::g_fps_counter.clear(); });

    conf.bind_setter<std::string>(
        "core.font.quality", &convert_and_set_requested_font_quality);

    std::ifstream ifs{config_file.native()};
    conf.load(ifs, filepathutil::to_utf8_path(config_file), false);

    if (Config::instance().run_wait < 1)
    {
        Config::instance().run_wait = 1;
    }
    if (Config::instance().attack_wait < 1)
    {
        Config::instance().attack_wait = 1;
    }
    if (Config::instance().start_run_wait >= 20)
    {
        Config::instance().start_run_wait = 1000;
    }
    if (Config::instance().language == spec::unknown_enum_variant)
    {
        config_query_language();
    }
    if (Config::instance().language == "jp")
    {
        jp = 1;
    }
    else
    {
        en = 1;
    }
}



void initialize_config_preload()
{
    const fs::path config_file = filesystem::files::profile_local_config();

    auto& conf = Config::instance();

    inject_display_modes(conf);
    inject_languages(conf);
    inject_save_files(conf);

    // clang-format off
    CONFIG_OPTION("language.language"s, std::string, Config::instance().language);
    CONFIG_OPTION("screen.fullscreen"s, std::string, Config::instance().fullscreen);
    CONFIG_OPTION("screen.music"s, bool, Config::instance().music);
    CONFIG_OPTION("screen.sound"s, bool, Config::instance().sound);
    CONFIG_OPTION("balance.extra_race"s, bool, Config::instance().extra_race);
    CONFIG_OPTION("balance.extra_class"s, bool, Config::instance().extra_class);
    CONFIG_OPTION("input.joypad"s, bool, Config::instance().joypad);
    CONFIG_OPTION("input.key_wait"s, json5::integer_type, Config::instance().key_wait);
    CONFIG_OPTION("game.default_save"s, std::string, defload); // TODO runtime enum
    CONFIG_OPTION("debug.wizard"s, bool, Config::instance().wizard);
    CONFIG_OPTION("screen.display_mode"s, std::string, Config::instance().display_mode);
    //clang-format on

    conf.bind_setter<int>(
        "core.android.quick_action_size", &set_touch_quick_action_size);

    conf.bind_setter<int>(
        "core.android.quick_action_transparency",
        &set_touch_quick_action_transparency);

    if (!fs::exists(config_file))
    {
        write_default_config(config_file);
    }

    std::ifstream ifs{
        config_file.native()};
    conf.load(ifs, filepathutil::to_utf8_path(config_file), true);

    snail::android::set_navigation_bar_visibility(
        !conf.get<bool>("core.android.hide_navigation"));

    // TODO: move it somewhere else or make it constant. "inf_tiles" is too
    // frequently used to find out where it should be initialized. Thus, it is
    // initialized as close to the previous position as possilbe.
    inf_tiles = 48;
}

#undef CONFIG_OPTION
#undef CONFIG_KEY


snail::Window::FullscreenMode config_get_fullscreen_mode()
{
    if (Config::instance().fullscreen == "fullscreen")
    {
        return snail::Window::FullscreenMode::fullscreen;
    }
    else if (Config::instance().fullscreen == "desktop_fullscreen")
    {
        return snail::Window::FullscreenMode::fullscreen_desktop;
    }
    else
    {
        return snail::Window::FullscreenMode::windowed;
    }
}



void Config::load_def(std::istream& is, const std::string& mod_id)
{
    def.load(is, "[input stream]", mod_id);
    mod_ids_.emplace(mod_id);
}



void Config::load_def(const fs::path& config_def_path, const std::string& mod_id)
{
    def.load(config_def_path, mod_id);
    mod_ids_.emplace(mod_id);
}



void Config::load_defaults(bool preload)
{
    for (auto& pair : def)
    {
        const std::string& key = pair.first;

        // Sections don't have defaults, so trying to set them would
        // cause an error.
        if (!def.is<spec::SectionDef>(key))
        {
            if (preload == def.get_metadata(key).preload)
            {
                const auto hcl_value = def.get_default(key);
                json5::value json_value;
                switch (hcl_value.type())
                {
                case hcl::Value::Type::BOOL_TYPE:
                    json_value = hcl_value.as<bool>();
                    break;
                case hcl::Value::Type::INT_TYPE:
                    json_value = hcl_value.as<int>();
                    break;
                case hcl::Value::Type::DOUBLE_TYPE:
                    json_value = hcl_value.as<double>();
                    break;
                case hcl::Value::Type::STRING_TYPE:
                    json_value = hcl_value.as<std::string>();
                    break;
                default:
                    assert(0);
                    break;
                }
                set(key, json_value);
            }
        }
    }
}



void Config::load(std::istream& is, const std::string& config_filename, bool preload)
{
    load_defaults(preload);

    std::string file_content{std::istreambuf_iterator<char>{is}, std::istreambuf_iterator<char>{}};
    json5::value value;
    try
    {
        value = json5::parse(file_content);
    }
    catch (json5::syntax_error& err)
    {
        std::cerr << err.what() << std::endl;
        throw ConfigLoadingError{u8"Failed to read " + config_filename + u8": " + err.what()};
    }

    // TODO: This pattern seems to be shared in various places in the
    // code.
    if (!value.is_object())
    {
        throw ConfigLoadingError(
            config_filename + ": \"config\" object not found at top level");
    }

    for (const auto& pair : value.get_object())
    {
        const auto& mod_id = pair.first;
        const auto& mod_section = pair.second;

        if (!mod_section.is_object())
        {
            continue;
        }

        visit_object(mod_section.get_object(), mod_id, config_filename, preload);
    }
}



void Config::visit_object(
    const json5::value::object_type& object,
    const std::string& current_key,
    const std::string& config_filename,
    bool preload)
{
    for (const auto& pair : object)
    {
        visit(pair.second, current_key + "." + pair.first, config_filename, preload);
    }
}



void Config::visit(
    const json5::value& value,
    const std::string& current_key,
    const std::string& config_filename,
    bool preload)
{
    if (value.is_object())
    {
        if (def.is<spec::SectionDef>(current_key))
        {
            visit_object(
                value.get_object(), current_key, config_filename, preload);
        }
    }
    else
    {
        if (def.exists(current_key))
        {
            if (preload == def.get_metadata(current_key).preload)
            {
                set(current_key, value);
            }
        }
    }
}



bool Config::verify_types(
    const json5::value& value,
    const std::string& current_key)
{
    if (def.is<spec::SectionDef>(current_key))
    {
        // It doesn't make sense to set a section as a value.
        return false;
    }
    if (value.is_boolean())
    {
        return def.is<spec::BoolDef>(current_key);
    }
    if (value.is_integer())
    {
        return def.is<spec::IntDef>(current_key);
    }
    if (value.is_array())
    {
        return def.is<spec::ListDef>(current_key);
    }
    if (value.is_string())
    {
        if (def.is<spec::EnumDef>(current_key))
        {
            auto EnumDef = def.get<spec::EnumDef>(current_key);
            if (EnumDef.pending)
            {
                // The key could be anything because the values are
                // not known yet, so don't attempt to check anything.
                return true;
            }
            else
            {
                return static_cast<bool>(
                    EnumDef.get_index_of(value.get_string()));
            }
        }
        else
        { return def.is<spec::StringDef>(current_key);
        }
    }

    return false;
}



void Config::save()
{
    json5::value::object_type object;

    // Create sections under the top-level object for each mod that
    // has config options, then write their individual config sections.
    for (auto&& pair : storage_)
    {
        std::string key = pair.first;
        json5::value value = pair.second;

        // Don't save hidden options if their value is the same as the default.
        if (!def.get_metadata(key).is_visible())
        {
            const auto hcl_value = def.get_default(key);
            switch (hcl_value.type())
            {
            case hcl::Value::Type::BOOL_TYPE:
                if (value.get_boolean() == hcl_value.as<bool>())
                {
                    continue;
                }
                break;
            case hcl::Value::Type::INT_TYPE:
                if (value.get_integer() == hcl_value.as<int>())
                {
                    continue;
                }
                break;
            case hcl::Value::Type::DOUBLE_TYPE:
                if (value.get_number() == hcl_value.as<double>())
                {
                    continue;
                }
                break;
            case hcl::Value::Type::STRING_TYPE:
                if (value.get_string() == hcl_value.as<std::string>())
                {
                    continue;
                }
                break;
            default:
                assert(0);
                continue;
            }
        }

        // Don't save injected enum values that are still unknown
        // (though this should never happen)
        if (def.is<spec::EnumDef>(key)
            && value.get_string() == spec::unknown_enum_variant)
        {
            continue;
        }

        size_t pos = 0;
        std::string token;
        json5::value::object_type* current = &object;

        // Function to split the flat key ("core.some.option")
        // on the next period and set the token to the split section
        // name ("some" or "option").
        auto advance = [&pos, &key, &token]() {
            pos = key.find(".");
            if (pos == std::string::npos)
            {
                return false;
            }
            token = key.substr(0, pos);
            key.erase(0, pos + 1);
            return true;
        };

        // Function that either creates a new object for holding the
        // nested config value or finds an existing one.
        auto set = [&current](std::string key) {
            const auto itr = current->find(key);
            if (itr != std::end(*current))
            {
                current = &itr->second.get_object();
            }
            else
            {
                current->emplace(key, json5::value::object_type{});
                current = &current->find(key)->second.get_object();
                assert(current);
            }
        };

        // Get the mod-level scope ("core").
        {
            const auto ok = advance();
            assert(ok);
        }
        std::string scope = token;
        set(token);

        // Traverse the remaining namespaces ("some.option").
        while (advance())
        {
            set(token);
        }

        // Set the value in the bottommost object ("some { option = 'value' }")
        current->emplace(key, value);
    }

    std::ofstream file{filesystem::files::profile_local_config().native(), std::ios::binary};
    if (!file)
    {
        throw ConfigLoadingError{
            u8"Failed to open: "s
            + filepathutil::make_preferred_path_in_utf8(
                  filesystem::files::profile_local_config())};
    }
    json5::stringify_options opts;
    opts.prettify = true;
    opts.sort_by_key = true;
    opts.unquote_key = true;
    opts.insert_trailing_comma = true;
    file << json5::stringify(object, opts);
}



void Config::clear()
{
    def.clear();
    storage_.clear();
    getters_.clear();
    setters_.clear();
}



void Config::inject_enum(
    const std::string& key,
    std::vector<std::string> variants,
    std::string default_variant)
{
    def.inject_enum(key, variants, default_variant);

    auto EnumDef = def.get<spec::EnumDef>(key);
    if (storage_.find(key) != storage_.end())
    {
        // Check if this enum has an invalid value. If so, set it to the
        // default.
        std::string current = get<std::string>(key);
        if (!EnumDef.get_index_of(current))
        {
            ELONA_WARN("config")
                << "Config key "s << key << " had invalid variant "s
                << current << ". "s
                << "("s << def.type_to_string(key) << ")"s
                << "Setting to "s << EnumDef.get_default() << "."s;
            set(key, EnumDef.get_default());
        }
    }
    else
    {
        set(key,
            EnumDef.get_default()); // Set the enum to its default value.
    }
}



bool Config::check_type(const std::string& key, json5::value_type type) const
{
    const auto itr = storage_.find(key);
    if (itr == storage_.end())
    {
        throw std::runtime_error("No such config value " + key);
    }
    return itr->second.type() == type;
}



void Config::set(const std::string& key, const json5::value& value)
{
    ELONA_LOG("config")
        << "Set: " << key << " to " << json5::stringify(value);

    if (!def.exists(key))
    {
        throw std::runtime_error("No such config key " + key);
    }
    if (verify_types(value, key))
    {
        if (value.is_integer())
        {
            int temp = value.get_integer();
            temp = clamp(temp, def.get_min(key), def.get_max(key));
            storage_[key] = temp;
        }
        else
        {
            storage_[key] = value;
        }

        if (setters_.find(key) != setters_.end())
        {
            setters_[key](storage_.at(key));
        }
    }
    else
    {
        std::stringstream ss;
        ss << "Wrong config item type for key " << key << ": ";
        ss << def.type_to_string(key) << " expected, got ";
        ss << json5::stringify(value);
        throw std::runtime_error(ss.str());
    }
}



void Config::run_setter(const std::string& key)
{
    if (storage_.find(key) == storage_.end())
    {
        return;
    }
    if (setters_.find(key) != setters_.end())
    {
        setters_[key](storage_.at(key));
    }
}

} // namespace elona
