#include "race.hpp"

#include "ability.hpp"
#include "character.hpp"
#include "data/types/type_ability.hpp"
#include "elona.hpp"
#include "game.hpp"
#include "i18n.hpp"
#include "random.hpp"
#include "variables.hpp"



namespace elona
{

void race_init_chara(Character& chara, data::InstanceId race_id)
{
    auto data = the_race_db[race_id];
    if (!data)
        return;

    chara.race = race_id;

    chara.melee_attack_type = data->melee_attack_type;
    chara.special_attack_type = data->special_attack_type;
    chara.dv_correction_value = data->dv_multiplier;
    chara.pv_correction_value = data->pv_multiplier;

    chara.birth_year = game()->date.year -
        (rnd(data->max_age - data->min_age + 1) + data->min_age);
    chara.height = data->height;

    if (mode == 1)
    {
        chara.sex = cmsex;
    }
    else
    {
        chara.sex = (rnd(100) < data->male_ratio) ? 0 : 1;
    }

    chara.image = (chara.sex == 0) ? data->male_image : data->female_image;

    if (data->is_made_of_rock)
    {
        chara.breaks_into_debris() = true;
    }

    {
        size_t index{};
        for (const auto& limb : data->body_parts)
        {
            chara.equipment_slots[index] = EquipmentSlot{limb, nullptr};
            ++index;
        }
        chara.equipment_slots[index] = EquipmentSlot{10, nullptr};
        ++index;
        chara.equipment_slots[index] = EquipmentSlot{11, nullptr};
        ++index;
    }

    for (const auto& pair : data->skills)
    {
        if (const auto ability_data = the_ability_db[pair.first])
        {
            chara_init_skill(chara, ability_data->integer_id, pair.second);
        }
        else
        {
            // Skip the skill if undefined.
            ELONA_WARN("lua.data") << "Undefined skill ID: " << pair.first.get()
                                   << " (race " << race_id.get() << ")";
        }
    }

    for (const auto& pair : data->resistances)
    {
        if (const auto ability_data = the_ability_db[pair.first])
        {
            chara.get_skill(ability_data->integer_id).level = pair.second;
        }
        else
        {
            // Skip the resistance if undefined.
            ELONA_WARN("lua.data")
                << "Undefined resistance ID: " << pair.first.get() << " (race "
                << race_id.get() << ")";
        }
    }
}



std::vector<std::reference_wrapper<const RaceData>> race_get_available(
    bool is_extra_race)
{
    std::vector<std::reference_wrapper<const RaceData>> ret;
    for (const auto& race : the_race_db.values())
    {
        if (race.is_extra == is_extra_race)
        {
            ret.emplace_back(race);
        }
    }
    range::sort(ret, [](const auto& a, const auto& b) {
        return a.get().ordering < b.get().ordering;
    });
    return ret;
}



void gain_race_feat()
{
    if (cdata.player().race == "core.dwarf")
    {
        cdata.player().traits().set_level("core.poison_resistance_nature", 2);
        cdata.player().traits().set_level("core.darkness_resistance_nature", 1);
    }
    if (cdata.player().race == "core.elea")
    {
        cdata.player().traits().set_level(
            "core.slow_ether_disease_progress", 1);
        cdata.player().traits().set_level("core.less_mana_reaction", 1);
    }
    if (cdata.player().race == "core.eulderna")
    {
        cdata.player().traits().set_level("core.magic_resistance_nature", 1);
    }
    if (cdata.player().race == "core.lich")
    {
        cdata.player().traits().set_level("core.cold_resistance_nature", 1);
        cdata.player().traits().set_level("core.darkness_resistance_nature", 2);
        cdata.player().traits().set_level("core.poison_resistance_nature", 1);
    }
    if (cdata.player().race == "core.golem")
    {
        cdata.player().traits().set_level("core.will_not_dimmed", 1);
        cdata.player().traits().set_level("core.poison_resistance_nature", 2);
    }
    if (cdata.player().race == "core.yerles")
    {
        cdata.player().traits().set_level("core.more_bonus_points", 1);
    }
    if (cdata.player().race == "core.juere")
    {
        cdata.player().traits().set_level("core.slow_digestion", 1);
        cdata.player().traits().set_level("core.more_materials", 1);
    }
    if (cdata.player().race == "core.goblin")
    {
        cdata.player().traits().set_level("core.darkness_resistance_nature", 1);
        cdata.player().traits().set_level("core.more_materials", 1);
    }
    if (cdata.player().race == "core.mutant")
    {
        cdata.player().traits().set_level("core.changing_body", 1);
    }
    if (cdata.player().race == "core.fairy")
    {
        cdata.player().traits().set_level("core.outstanding_resistances", 1);
        cdata.player().traits().set_level(
            "core.cannot_wear_heavy_equipments", 1);
    }
}

} // namespace elona
