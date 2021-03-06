#include "ctrl_file.hpp"

#include <set>

#include "../util/fileutil.hpp"
#include "ability.hpp"
#include "area.hpp"
#include "character.hpp"
#include "character_status.hpp"
#include "data/types/type_ability.hpp"
#include "data/types/type_blending_recipe.hpp"
#include "data/types/type_crafting_material.hpp"
#include "deferred_event.hpp"
#include "elona.hpp"
#include "filesystem.hpp"
#include "game.hpp"
#include "item.hpp"
#include "log.hpp"
#include "lua_env/handle_manager.hpp"
#include "lua_env/lua_env.hpp"
#include "lua_env/mod_serializer.hpp"
#include "map.hpp"
#include "mef.hpp"
#include "quest.hpp"
#include "save_fs.hpp"
#include "save_header.hpp"
#include "serialization/serialization.hpp"
#include "serialization/utils.hpp"
#include "variables.hpp"



namespace elona
{

namespace
{

void arrayfile_read(const std::string& fmode_str, const fs::path& filepath)
{
    std::vector<std::string> lines;
    if (save_fs_exists(filepath.filename()))
    {
        range::copy(
            fileutil::read_by_line(filepath), std::back_inserter(lines));
    }

    if (fmode_str == u8"qname"s)
    {
        lines.resize(500);
        auto itr = std::begin(lines);
        for (int i = 0; i < 500; ++i)
        {
            qname(i) = *itr;
            ++itr;
        }
    }
    else if (fmode_str == u8"mdatan"s)
    {
        lines.resize(2);
        auto itr = std::begin(lines);
        for (int i = 0; i < 2; ++i)
        {
            mdatan(i) = *itr;
            ++itr;
        }
    }
}



void arrayfile_write(const std::string& fmode_str, const fs::path& filepath)
{
    std::ofstream out{filepath.native(), std::ios::binary};
    if (!out)
    {
        throw std::runtime_error(
            u8"Error: fail to write " + filepath.to_u8string());
    }

    if (fmode_str == u8"qname"s)
    {
        for (int i = 0; i < 500; ++i)
        {
            out << qname(i) << std::endl;
        }
    }
    else if (fmode_str == u8"mdatan"s)
    {
        for (int i = 0; i < 2; ++i)
        {
            out << mdatan(i) << std::endl;
        }
    }

    save_fs_add(filepath.filename());
}



void arrayfile(
    bool fread,
    const std::string& fmode_str,
    const fs::path& filepath)
{
    if (!fread)
    {
        arrayfile_write(fmode_str, filepath);
    }
    else
    {
        arrayfile_read(fmode_str, filepath);
    }
}



template <typename F>
void load_internal(const fs::path& filepath, F do_load)
{
    std::ifstream in{filepath.native(), std::ios::binary};
    if (in.fail())
    {
        throw std::runtime_error(
            u8"Could not open file at "s + filepath.to_u8string());
    }
    serialization::binary::IArchive ar{in};
    do_load(ar);
}



template <typename F>
void save_internal(const fs::path& filepath, F do_save)
{
    std::ofstream out{filepath.native(), std::ios::binary};
    if (out.fail())
    {
        throw std::runtime_error(
            u8"Could not open file at "s + filepath.to_u8string());
    }
    serialization::binary::OArchive ar{out};
    do_save(ar);
}



template <typename T>
void load_v1(
    const fs::path& filepath,
    elona_vector1<T>& data,
    size_t begin,
    size_t end)
{
    load_internal(filepath, [&](auto& ar) {
        for (size_t i = begin; i < end; ++i)
        {
            ar(data(i));
        }
    });
}


template <typename T>
void save_v1(
    const fs::path& filepath,
    elona_vector1<T>& data,
    size_t begin,
    size_t end)
{
    save_internal(filepath, [&](auto& ar) {
        for (size_t i = begin; i < end; ++i)
        {
            ar(data(i));
        }
    });
}


template <typename T>
void load_v2(
    const fs::path& filepath,
    elona_vector2<T>& data,
    size_t i_begin,
    size_t i_end,
    size_t j_begin,
    size_t j_end)
{
    load_internal(filepath, [&](auto& ar) {
        for (size_t j = j_begin; j < j_end; ++j)
        {
            for (size_t i = i_begin; i < i_end; ++i)
            {
                ar(data(i, j));
            }
        }
    });
}


template <typename T>
void save_v2(
    const fs::path& filepath,
    elona_vector2<T>& data,
    size_t i_begin,
    size_t i_end,
    size_t j_begin,
    size_t j_end)
{
    save_internal(filepath, [&](auto& ar) {
        for (size_t j = j_begin; j < j_end; ++j)
        {
            for (size_t i = i_begin; i < i_end; ++i)
            {
                ar(data(i, j));
            }
        }
    });
}


template <typename T>
void load_v3(
    const fs::path& filepath,
    elona_vector3<T>& data,
    size_t i_begin,
    size_t i_end,
    size_t j_begin,
    size_t j_end,
    size_t k_begin,
    size_t k_end)
{
    load_internal(filepath, [&](auto& ar) {
        for (size_t k = k_begin; k < k_end; ++k)
        {
            for (size_t j = j_begin; j < j_end; ++j)
            {
                for (size_t i = i_begin; i < i_end; ++i)
                {
                    ar(data(i, j, k));
                }
            }
        }
    });
}


template <typename T>
void save_v3(
    const fs::path& filepath,
    elona_vector3<T>& data,
    size_t i_begin,
    size_t i_end,
    size_t j_begin,
    size_t j_end,
    size_t k_begin,
    size_t k_end)
{
    save_internal(filepath, [&](auto& ar) {
        for (size_t k = k_begin; k < k_end; ++k)
        {
            for (size_t j = j_begin; j < j_end; ++j)
            {
                for (size_t i = i_begin; i < i_end; ++i)
                {
                    ar(data(i, j, k));
                }
            }
        }
    });
}


template <typename T>
void load_vec(const fs::path& filepath, std::vector<T>& data)
{
    load_internal(filepath, [&](auto& ar) {
        for (auto&& element : data)
        {
            ar(element);
        }
    });
}


template <typename T>
void load(const fs::path& filepath, T& data, size_t begin, size_t end)
{
    load_internal(filepath, [&](auto& ar) {
        for (size_t i = begin; i < end; ++i)
        {
            ar(data[i]);
        }
    });
}


template <typename T>
void save(const fs::path& filepath, T& data, size_t begin, size_t end)
{
    save_internal(filepath, [&](auto& ar) {
        for (size_t i = begin; i < end; ++i)
        {
            ar(data[i]);
        }
    });
}


template <typename T>
void load(const fs::path& filepath, T& data)
{
    load_internal(filepath, [&](auto& ar) { ar(data); });
}


template <typename T>
void save(const fs::path& filepath, T& data)
{
    save_internal(filepath, [&](auto& ar) { ar(data); });
}



void restore_handles(int index_start, int index_end)
{
    sol::table obj_ids = lua::lua->get_state()->create_table();
    for (int index = index_start; index < index_end; ++index)
    {
        auto& obj = cdata[index];
        const auto key = reinterpret_cast<int64_t>(std::addressof(obj));
        obj_ids[key] = obj.obj_id.to_string();
    }

    auto& handle_mgr = lua::lua->get_handle_manager();
    handle_mgr.merge_handles(Character::lua_type(), obj_ids);

    ELONA_LOG("lua.mod") << "Loaded handle data for " << Character::lua_type()
                         << " in [" << index_start << "," << index_end << "]";

    for (int index = index_start; index < index_end; index++)
    {
        handle_mgr.resolve_handle(cdata[index]);
    }
}



bool will_resolve_object_reference = false;

void schedule_object_reference_resolution()
{
    will_resolve_object_reference = true;
}



OptionalItemRef resolve_object_reference(const ObjId& obj_id)
{
    return ItemIdTable::instance().get(obj_id);
}



void clear_pending_ids()
{
    eobject::internal::_pending_ids<Item>.clear();
}



void resolve_pending_ids()
{
    for (auto& [ref_ptr, obj_id] : eobject::internal::_pending_ids<Item>)
    {
        *ref_ptr = resolve_object_reference(obj_id);
    }
}



void inventory_deserialize(serialization::binary::IArchive& ar, Inventory& inv)
{
    inv.clear();
    const auto n = inv.size();
    for (size_t i = 0; i < n; ++i)
    {
        const auto slot = static_cast<InventorySlot>(i);
        bool exists;
        ar(exists);
        if (exists)
        {
            const auto item_ref = inv.create(slot);
            auto& item = *item_ref.get_raw_ptr();
            ar(item);
            ItemIdTable::instance().add(item_ref);
        }
    }
}



void inventory_serialize(serialization::binary::OArchive& ar, Inventory& inv)
{
    const auto n = inv.size();
    for (size_t i = 0; i < n; ++i)
    {
        const auto slot = static_cast<InventorySlot>(i);
        bool exists = !!inv.at(slot);
        ar(exists);
        if (exists)
        {
            const auto item_ref = inv.at(slot).unwrap();
            auto& item = *item_ref.get_raw_ptr();
            ar(item);
        }
    }
}

} // namespace



void ctrl_file_global_read(const fs::path& dir)
{
    ELONA_LOG("save.ctrl_file")
        << "global_read(" << dir.to_u8string() << ") BEGIN";

    game()->play_time = game()->play_time + timeGetTime() / 1000 - time_begin;
    time_begin = timeGetTime() / 1000;

    {
        const auto filepath = dir / u8"gdata.s1";
        load_v1(filepath, gdata, 0, 1000);
        game()->unpack_from(gdata);
    }

    {
        const auto filepath = dir / u8"cdata.s1";
        load(filepath, cdata, 0, ELONA_MAX_PARTY_CHARACTERS);

        restore_handles(0, ELONA_MAX_PARTY_CHARACTERS);
    }

    {
        const auto filepath = dir / u8"spell.s1";
        elona_vector1<int> spell;
        DIM2(spell, 200);
        load_v1(filepath, spell, 0, 200);
        cdata.player().spell_stocks().unpack_from(spell);
    }

    {
        const auto filepath = dir / u8"inv.s1";
        load_internal(filepath, [&](auto& ar) {
            for (auto&& inv : g_inv.global())
            {
                inventory_deserialize(ar, inv);
            }
        });
    }

    {
        const auto filepath = dir / u8"kitem.s1";
        elona_vector2<int> itemmemory;
        DIM3(itemmemory, 3, 800);
        load_v2(filepath, itemmemory, 0, 3, 0, 800);
        game()->item_memories().unpack_from(itemmemory);
    }

    {
        const auto filepath = dir / u8"knpc.s1";
        elona_vector2<int> npcmemory;
        DIM3(npcmemory, 2, 800);
        load_v2(filepath, npcmemory, 0, 2, 0, 800);
        game()->character_memories().unpack_from(npcmemory);
    }

    {
        const auto filepath = dir / u8"adata.s1";
        load_v2(filepath, adata, 0, 40, 0, 500);
        area_data.unpack_from(adata);
    }

    {
        const auto filepath = dir / u8"spact.s1";
        elona_vector1<int> spact;
        DIM2(spact, 500);
        load_v1(filepath, spact, 0, 500);
        cdata.player().spacts().unpack_from(spact);
    }

    {
        const auto filepath = dir / u8"qdata.s1";
        load_v2(filepath, qdata, 0, 20, 0, 500);
        quest_data.unpack_from(qdata);
    }

    {
        const auto filepath = dir / u8"mat.s1";
        elona_vector1<int> mat;
        DIM2(mat, 400);
        load_v1(filepath, mat, 0, 400);
        game()->crafting_materials().unpack_from(mat);
    }

    {
        const auto filepath = dir / u8"trait.s1";
        elona_vector1<int> trait;
        DIM2(trait, 500);
        load_v1(filepath, trait, 0, 500);
        cdata.player().traits().unpack_from(trait);
    }

    {
        const auto filepath = dir / u8"pcc.s1";
        load_v2(filepath, pcc, 0, 30, 0, 20);
    }

    {
        const auto filepath = dir / u8"card.s1";
        load_v2(filepath, card, 0, 100, 0, 40);
    }

    {
        const auto filepath = dir / u8"krecipe.s1";
        elona_vector1<int> recipememory;
        DIM2(recipememory, 1200);
        load_v1(filepath, recipememory, 0, 1200);
        game()->blending_recipe_memories().unpack_from(recipememory);
    }

    {
        const auto filepath = dir / u8"art.log";
        artifactlocation.clear();
        range::copy(
            fileutil::read_by_line(filepath),
            std::back_inserter(artifactlocation));
    }

    {
        const auto filepath = dir / u8"news.log";
        notesel(newsbuff);
        newsbuff(0).clear();
        std::ifstream in{filepath.native(), std::ios::binary};
        std::string tmp;
        while (std::getline(in, tmp))
        {
            newsbuff(0) += tmp + '\n';
        }
    }

    arrayfile(true, u8"qname", dir / u8"qname.s1");

    if (fs::exists(dir / "deferred_events.s1"))
    {
        event_load(dir / "deferred_events.s1");
    }

    lua::ModSerializer mod_serializer(*lua::lua);
    lua::lua->get_mod_manager().clear_global_stores();

    {
        const auto filepath = dir / u8"mod.s1";
        std::ifstream in{filepath.native(), std::ios::binary};
        serialization::binary::IArchive ar{in};
        mod_serializer.load_mod_store_data(ar, lua::ModEnv::StoreType::global);
    }

    resolve_pending_ids();
    // References to map-local objects have not been resolved.
    // clear_pending_ids();

    ELONA_LOG("save.ctrl_file")
        << "global_read(" << dir.to_u8string() << ") END";
}



void ctrl_file_global_write(const fs::path& dir)
{
    ELONA_LOG("save.ctrl_file")
        << "global_write(" << dir.to_u8string() << ") BEGIN";

    game()->play_time = game()->play_time + timeGetTime() / 1000 - time_begin;
    time_begin = timeGetTime() / 1000;

    if (!fs::exists(dir))
    {
        fs::create_directory(dir);
    }

    SaveHeader::save(dir);

    {
        const auto filepath = dir / u8"version.s0";
        auto v = latest_version;
        serialization::binary::save(filepath, v);
    }

    {
        const auto filepath = dir / u8"gdata.s1";
        game()->pack_to(gdata);
        save_v1(filepath, gdata, 0, 1000);
    }

    {
        const auto filepath = dir / u8"cdata.s1";
        save(filepath, cdata, 0, ELONA_MAX_PARTY_CHARACTERS);
    }

    {
        const auto filepath = dir / u8"spell.s1";
        elona_vector1<int> spell;
        DIM2(spell, 200);
        cdata.player().spell_stocks().pack_to(spell);
        save_v1(filepath, spell, 0, 200);
    }

    {
        const auto filepath = dir / u8"inv.s1";
        save_internal(filepath, [&](auto& ar) {
            for (auto&& inv : g_inv.global())
            {
                inventory_serialize(ar, inv);
            }
        });
    }

    {
        const auto filepath = dir / u8"kitem.s1";
        elona_vector2<int> itemmemory;
        DIM3(itemmemory, 3, 800);
        game()->item_memories().pack_to(itemmemory);
        save_v2(filepath, itemmemory, 0, 3, 0, 800);
    }

    {
        const auto filepath = dir / u8"knpc.s1";
        elona_vector2<int> npcmemory;
        DIM3(npcmemory, 2, 800);
        game()->character_memories().pack_to(npcmemory);
        save_v2(filepath, npcmemory, 0, 2, 0, 800);
    }

    {
        const auto filepath = dir / u8"adata.s1";
        area_data.pack_to(adata);
        save_v2(filepath, adata, 0, 40, 0, 500);
    }

    {
        const auto filepath = dir / u8"spact.s1";
        elona_vector1<int> spact;
        DIM2(spact, 500);
        cdata.player().spacts().pack_to(spact);
        save_v1(filepath, spact, 0, 500);
    }

    {
        const auto filepath = dir / u8"qdata.s1";
        quest_data.pack_to(qdata);
        save_v2(filepath, qdata, 0, 20, 0, 500);
    }

    {
        const auto filepath = dir / u8"mat.s1";
        elona_vector1<int> mat;
        DIM2(mat, 400);
        game()->crafting_materials().pack_to(mat);
        save_v1(filepath, mat, 0, 400);
    }

    {
        const auto filepath = dir / u8"trait.s1";
        elona_vector1<int> trait;
        DIM2(trait, 500);
        cdata.player().traits().pack_to(trait);
        save_v1(filepath, trait, 0, 500);
    }

    {
        const auto filepath = dir / u8"pcc.s1";
        save_v2(filepath, pcc, 0, 30, 0, 20);
    }

    {
        const auto filepath = dir / u8"card.s1";
        save_v2(filepath, card, 0, 100, 0, 40);
    }

    {
        const auto filepath = dir / u8"krecipe.s1";
        elona_vector1<int> recipememory;
        DIM2(recipememory, 1200);
        game()->blending_recipe_memories().pack_to(recipememory);
        save_v1(filepath, recipememory, 0, 1200);
    }

    {
        const auto filepath = dir / u8"art.log";
        std::ofstream out{filepath.native(), std::ios::binary};
        range::for_each(artifactlocation, [&](const auto& line) {
            out << line << std::endl;
        });
    }

    {
        const auto filepath = dir / u8"news.log";
        notesel(newsbuff);
        std::ofstream out{filepath.native(), std::ios::binary};
        out << newsbuff(0) << std::endl;
    }

    arrayfile(false, u8"qname", dir / u8"qname.s1");

    event_save(dir / "deferred_events.s1");

    lua::ModSerializer mod_serializer(*lua::lua);

    {
        const auto filepath = dir / u8"mod.s1";
        std::ofstream out{filepath.native(), std::ios::binary};
        serialization::binary::OArchive ar{out};
        mod_serializer.save_mod_store_data(ar, lua::ModEnv::StoreType::global);
    }

    ELONA_LOG("save.ctrl_file")
        << "global_write(" << dir.to_u8string() << ") END";
}



void ctrl_file_map_read()
{
    ELONA_LOG("save.ctrl_file") << "map_read() BEGIN";

    const auto dir = filesystem::dirs::tmp();

    {
        const auto filepath = dir / fs::u8path(u8"mdata_"s + mid + u8".s2");
        (void)save_fs_exists(fs::u8path(u8"mdata_"s + mid + u8".s2"));
        load_v1(filepath, mdata, 0, 100);
        map_data.unpack_from(mdata);
    }

    {
        const auto filepath = dir / fs::u8path(u8"map_"s + mid + u8".s2");
        DIM3(mapsync, map_data.width, map_data.height);
        DIM3(mef, 9, MEF_MAX);
        (void)save_fs_exists(fs::u8path(u8"map_"s + mid + u8".s2"));
        load(filepath, cell_data);
    }

    {
        const auto filepath = dir / fs::u8path(u8"cdata_"s + mid + u8".s2");
        (void)save_fs_exists(fs::u8path(u8"cdata_"s + mid + u8".s2"));
        load(filepath, cdata, ELONA_MAX_PARTY_CHARACTERS, ELONA_MAX_CHARACTERS);

        restore_handles(ELONA_MAX_PARTY_CHARACTERS, ELONA_MAX_CHARACTERS);
    }

    {
        const auto filepath = dir / fs::u8path(u8"mef_"s + mid + u8".s2");
        (void)save_fs_exists(fs::u8path(u8"mef_"s + mid + u8".s2"));
        load_v2(filepath, mef, 0, 9, 0, MEF_MAX);
    }

    arrayfile(true, u8"mdatan", dir / fs::u8path(u8"mdatan_"s + mid + u8".s2"));

    lua::ModSerializer mod_serializer(*lua::lua);
    lua::lua->get_mod_manager().clear_map_local_stores();

    // Mod map-local store data (Store.map)
    {
        const auto filepath = dir / fs::u8path(u8"mod_map_"s + mid + u8".s2");
        (void)save_fs_exists(fs::u8path(u8"mod_map_"s + mid + u8".s2"));

        std::ifstream in{filepath.native(), std::ios::binary};
        serialization::binary::IArchive ar{in};
        mod_serializer.load_mod_store_data(ar, lua::ModEnv::StoreType::map);
    }

    schedule_object_reference_resolution();

    ELONA_LOG("save.ctrl_file") << "map_read() END";
}



void ctrl_file_map_write()
{
    ELONA_LOG("save.ctrl_file") << "map_write() BEGIN";

    const auto dir = filesystem::dirs::tmp();

    {
        const auto filepath = dir / fs::u8path(u8"mdata_"s + mid + u8".s2");
        save_fs_add(filepath.filename());
        map_data.pack_to(mdata);
        save_v1(filepath, mdata, 0, 100);
    }

    {
        const auto filepath = dir / fs::u8path(u8"map_"s + mid + u8".s2");
        save_fs_add(filepath.filename());
        save(filepath, cell_data);
    }

    {
        const auto filepath = dir / fs::u8path(u8"cdata_"s + mid + u8".s2");
        save_fs_add(filepath.filename());
        save(filepath, cdata, ELONA_MAX_PARTY_CHARACTERS, ELONA_MAX_CHARACTERS);
    }

    {
        const auto filepath = dir / fs::u8path(u8"mef_"s + mid + u8".s2");
        save_fs_add(filepath.filename());
        save_v2(filepath, mef, 0, 9, 0, MEF_MAX);
    }

    arrayfile(
        false, u8"mdatan", dir / fs::u8path(u8"mdatan_"s + mid + u8".s2"));

    lua::ModSerializer mod_serializer(*lua::lua);

    // Mod map-local store data (Store.map)
    {
        const auto filepath = dir / fs::u8path(u8"mod_map_"s + mid + u8".s2");
        save_fs_add(filepath.filename());

        std::ofstream out{filepath.native(), std::ios::binary};
        serialization::binary::OArchive ar{out};
        mod_serializer.save_mod_store_data(ar, lua::ModEnv::StoreType::map);
    }

    ELONA_LOG("save.ctrl_file") << "map_write() END";
}



void ctrl_file_map_items_read(const fs::path& filename)
{
    ELONA_LOG("save.ctrl_file")
        << "map_items_read(" << filename.to_u8string() << ") BEGIN";

    const auto filepath = filesystem::dirs::tmp() / filename;
    (void)save_fs_exists(filename);

    load_internal(filepath, [&](auto& ar) {
        for (auto&& inv : g_inv.map_local())
        {
            inventory_deserialize(ar, inv);
        }
    });

    if (will_resolve_object_reference)
    {
        resolve_pending_ids();
        clear_pending_ids();
        will_resolve_object_reference = false;
    }

    ELONA_LOG("save.ctrl_file")
        << "map_items_read(" << filename.to_u8string() << ") END";
}



void ctrl_file_map_items_write(const fs::path& filename)
{
    ELONA_LOG("save.ctrl_file")
        << "map_items_write(" << filename.to_u8string() << ") BEGIN";

    const auto filepath = filesystem::dirs::tmp() / filename;
    save_fs_add(filepath.filename());
    (void)save_fs_exists(filename);

    save_internal(filepath, [&](auto& ar) {
        for (auto&& inv : g_inv.map_local())
        {
            inventory_serialize(ar, inv);
        }
    });

    ELONA_LOG("save.ctrl_file")
        << "map_items_write(" << filename.to_u8string() << ") END";
}



void ctrl_file_custom_map_read(const fs::path& filename_base)
{
    ELONA_LOG("save.ctrl_file")
        << "custom_map_read(" << filename_base.to_u8string() << ") BEGIN";

    DIM3(cmapdata, 5, 400);
    DIM3(mef, 9, MEF_MAX);

    {
        const auto filepath = filename_base.replace_extension("idx");
        load_v1(filepath, mdatatmp, 0, 100);
        for (int j = 0; j < 5; ++j)
        {
            mdata(j) = mdatatmp(j);
        }
        map_data.width = mdata(0);
        map_data.height = mdata(1);
        map_data.atlas_number = mdata(2);
        map_data.next_regenerate_date = mdata(3);
        map_data.stair_down_pos = mdata(4);
    }

    {
        const auto filepath = filename_base.replace_extension("map");
        DIM3(mapsync, map_data.width,
             map_data.height); // TODO length_exception
        cell_data.init(map_data.width, map_data.height);
        std::vector<int> tile_grid(map_data.width * map_data.height);
        load_vec(filepath, tile_grid);
        cell_data.load_tile_grid(tile_grid);
    }

    {
        const auto filepath = filename_base.replace_extension("obj");
        if (fs::exists(filepath))
        {
            load_v2(filepath, cmapdata, 0, 5, 0, 400);
        }
    }

    ELONA_LOG("save.ctrl_file")
        << "custom_map_read(" << filename_base.to_u8string() << ") END";
}



void ctrl_file_map_load_map_obj_files(const fs::path& filename_base)
{
    ELONA_LOG("save.ctrl_file") << "map_load_map_obj_files("
                                << filename_base.to_u8string() << ") BEGIN";

    DIM3(cmapdata, 5, 400);

    {
        const auto filepath = filename_base.replace_extension("map");
        std::vector<int> tile_grid(cell_data.width() * cell_data.height());
        load_vec(filepath, tile_grid);
        cell_data.load_tile_grid(tile_grid);
    }

    {
        const auto filepath = filename_base.replace_extension("obj");
        if (fs::exists(filepath))
        {
            load_v2(filepath, cmapdata, 0, 5, 0, 400);
        }
    }

    ELONA_LOG("save.ctrl_file")
        << "map_load_map_obj_files(" << filename_base.to_u8string() << ") END";
}



void ctrl_file_save_game_delete()
{
    ELONA_LOG("save.ctrl_file") << "save_game_delete() BEGIN";

    fs::remove_all(filesystem::dirs::save(playerid));

    ELONA_LOG("save.ctrl_file") << "save_game_delete() END";
}



void ctrl_file_temp_dir_delete()
{
    ELONA_LOG("save.ctrl_file") << "temp_dir_delete() BEGIN";

    for (const auto& entry : filesystem::glob_files(
             filesystem::dirs::tmp(), std::regex{u8R"(.*\..*)"}))
    {
        fs::remove_all(entry.path());
    }

    ELONA_LOG("save.ctrl_file") << "temp_dir_delete() END";
}



void ctrl_file_map_delete()
{
    ELONA_LOG("save.ctrl_file") << "map_delete() BEGIN";

    const auto filename = fs::u8path(u8"map_"s + mid + u8".s2");
    if (!save_fs_exists(filename))
        return;

    save_fs_remove(filename);
    save_fs_remove(fs::u8path("cdata_"s + mid + ".s2"));
    save_fs_remove(fs::u8path("inv_"s + mid + ".s2"));
    save_fs_remove(fs::u8path("mod_map_"s + mid + ".s2"));
    save_fs_remove(fs::u8path("mdata_"s + mid + ".s2"));
    save_fs_remove(fs::u8path("mdatan_"s + mid + ".s2"));
    save_fs_remove(fs::u8path("mef_"s + mid + ".s2"));

    ELONA_LOG("save.ctrl_file") << "map_delete() END";
}



void ctrl_file_map_delete_preserve_items()
{
    ELONA_LOG("save.ctrl_file") << "map_delete_preserve_items() BEGIN";

    if (!save_fs_exists(fs::u8path(u8"mdata_"s + mid + u8".s2")))
    {
        // We tried preserving the characters/items, but the home
        // map to transfer them from didn't exist.
        return;
    }
    const auto filename = fs::u8path(u8"map_"s + mid + u8".s2");
    if (!save_fs_exists(filename))
        return;

    save_fs_remove(filename);
    save_fs_remove(fs::u8path("mdata_"s + mid + ".s2"));
    save_fs_remove(fs::u8path("mdatan_"s + mid + ".s2"));
    save_fs_remove(fs::u8path("mef_"s + mid + ".s2"));

    ELONA_LOG("save.ctrl_file") << "map_delete_preserve_items() END";
}



void ctrl_file_temp_dir_delete_area()
{
    ELONA_LOG("save.ctrl_file") << "temp_dir_delete_area() BEGIN";

    area_data[area].clear();

    for (const auto& entry : filesystem::glob_files(
             filesystem::dirs::save(playerid),
             std::regex{u8R"(.*_)"s + area + u8R"(_.*\..*)"}))
    {
        save_fs_remove(entry.path().filename());
    }
    for (const auto& entry : filesystem::glob_files(
             filesystem::dirs::tmp(),
             std::regex{u8R"(.*_)"s + area + u8R"(_.*\..*)"}))
    {
        save_fs_remove(entry.path().filename());
    }

    ELONA_LOG("save.ctrl_file") << "temp_dir_delete_area() END";
}



void ctrl_file_map_home_upgrade()
{
    ELONA_LOG("save.ctrl_file") << "map_home_upgrade() BEGIN";

    const auto dir = filesystem::dirs::tmp();

    if (!fs::exists(dir / fs::u8path(u8"cdata_"s + mid + u8".s2")))
        return;

    {
        const auto filepath = dir / fs::u8path(u8"cdata_"s + mid + u8".s2");
        (void)save_fs_exists(fs::u8path(u8"cdata_"s + mid + u8".s2"));
        load(filepath, cdata, ELONA_MAX_PARTY_CHARACTERS, ELONA_MAX_CHARACTERS);
    }

    schedule_object_reference_resolution();

    ELONA_LOG("save.ctrl_file") << "map_home_upgrade() END";
}



void ctrl_file_deck_read(const fs::path& filename)
{
    ELONA_LOG("save.ctrl_file")
        << "deck_read(" << filename.to_u8string() << ") BEGIN";

    load_v1(filesystem::dirs::tmp() / filename, deck, 0, 1000);

    ELONA_LOG("save.ctrl_file")
        << "deck_read(" << filename.to_u8string() << ") END";
}



void ctrl_file_deck_write(const fs::path& filename)
{
    ELONA_LOG("save.ctrl_file")
        << "deck_write(" << filename.to_u8string() << ") BEGIN";

    save_fs_add(filename);
    save_v1(filesystem::dirs::tmp() / filename, deck, 0, 1000);

    ELONA_LOG("save.ctrl_file")
        << "deck_write(" << filename.to_u8string() << ") END";
}



void ctrl_file_tmp_inv_read(const fs::path& filename)
{
    ELONA_LOG("save.ctrl_file")
        << "tmp_inv_read(" << filename.to_u8string() << ") BEGIN";

    const auto path = filesystem::dirs::tmp() / filename;
    (void)save_fs_exists(filename);

    load_internal(
        path, [&](auto& ar) { inventory_deserialize(ar, *g_inv.tmp()); });

    ELONA_LOG("save.ctrl_file")
        << "tmp_inv_read(" << filename.to_u8string() << ") END";
}



void ctrl_file_tmp_inv_write(const fs::path& filename)
{
    ELONA_LOG("save.ctrl_file")
        << "tmp_inv_write(" << filename.to_u8string() << ") BEGIN";

    const auto path = filesystem::dirs::tmp() / filename;
    save_fs_add(filename);
    (void)save_fs_exists(filename);

    save_internal(
        path, [&](auto& ar) { inventory_serialize(ar, *g_inv.tmp()); });

    ELONA_LOG("save.ctrl_file")
        << "tmp_inv_write(" << filename.to_u8string() << ") END";
}



void SpellStockTable::pack_to(elona_vector1<int>& legacy_spell) const
{
    for (int i = 0; i < 200; ++i)
    {
        const auto integer_spell_id = i + 400;
        if (const auto id =
                the_ability_db.get_id_from_integer(integer_spell_id))
        {
            if (const auto itr = _stocks.find(*id); itr != _stocks.end())
            {
                legacy_spell(i) = itr->second;
            }
        }
    }
}



void SpellStockTable::unpack_from(elona_vector1<int>& legacy_spell)
{
    _stocks.clear();

    for (int i = 0; i < 200; ++i)
    {
        if (legacy_spell(i) == 0)
            continue;

        const auto integer_spell_id = i + 400;
        if (const auto id =
                the_ability_db.get_id_from_integer(integer_spell_id))
        {
            _stocks[*id] = legacy_spell(i);
        }
    }
}



void SpactTable::pack_to(elona_vector1<int>& legacy_spact) const
{
    for (int i = 0; i < 500; ++i)
    {
        const auto integer_spact_id = i + 600;
        if (const auto id =
                the_ability_db.get_id_from_integer(integer_spact_id))
        {
            if (_spacts.find(*id) != _spacts.end())
            {
                legacy_spact(i) = 1;
            }
        }
    }
}



void SpactTable::unpack_from(elona_vector1<int>& legacy_spact)
{
    _spacts.clear();

    for (int i = 0; i < 500; ++i)
    {
        if (legacy_spact(i) == 0)
            continue;

        const auto integer_spact_id = i + 600;
        if (const auto id =
                the_ability_db.get_id_from_integer(integer_spact_id))
        {
            _spacts.emplace(*id);
        }
    }
}



void TraitLevelTable::pack_to(elona_vector1<int>& legacy_trait) const
{
    for (int i = 0; i < 500; ++i)
    {
        const auto integer_trait_id = i;
        if (const auto id = the_trait_db.get_id_from_integer(integer_trait_id))
        {
            if (const auto itr = _traits.find(*id); itr != _traits.end())
            {
                legacy_trait(i) = itr->second;
            }
        }
    }
}



void TraitLevelTable::unpack_from(elona_vector1<int>& legacy_trait)
{
    _traits.clear();

    for (int i = 0; i < 500; ++i)
    {
        if (legacy_trait(i) == 0)
            continue;

        const auto integer_trait_id = i;
        if (const auto id = the_trait_db.get_id_from_integer(integer_trait_id))
        {
            _traits[*id] = legacy_trait(i);
        }
    }
}



void BlendingRecipeMemoryTable::pack_to(
    elona_vector1<int>& legacy_recipememory) const
{
    for (int i = 0; i < 1200; ++i)
    {
        const auto integer_recipe_id = i;
        if (const auto id =
                the_blending_recipe_db.get_id_from_integer(integer_recipe_id))
        {
            if (const auto itr = _memories.find(*id); itr != _memories.end())
            {
                legacy_recipememory(i) = itr->second.read_count;
            }
        }
    }
}



void BlendingRecipeMemoryTable::unpack_from(
    elona_vector1<int>& legacy_recipememory)
{
    _memories.clear();

    for (int i = 0; i < 1200; ++i)
    {
        if (legacy_recipememory(i) == 0)
            continue;

        const auto integer_recipe_id = i;
        if (const auto id =
                the_blending_recipe_db.get_id_from_integer(integer_recipe_id))
        {
            _memories[*id].read_count = legacy_recipememory(i);
        }
    }
}



void CharacterMemoryTable::pack_to(elona_vector2<int>& legacy_npcmemory) const
{
    for (int i = 0; i < 800; ++i)
    {
        const auto integer_chara_id = i;
        if (const auto id =
                the_character_db.get_id_from_integer(integer_chara_id))
        {
            if (const auto itr = _memories.find(*id); itr != _memories.end())
            {
                legacy_npcmemory(0, i) = itr->second.kill_count;
                legacy_npcmemory(1, i) = itr->second.generate_count;
            }
        }
    }
}



void CharacterMemoryTable::unpack_from(elona_vector2<int>& legacy_npcmemory)
{
    _memories.clear();

    for (int i = 0; i < 800; ++i)
    {
        if (legacy_npcmemory(0, i) == 0 && legacy_npcmemory(1, i) == 0)
            continue;

        const auto integer_chara_id = i;
        if (const auto id =
                the_character_db.get_id_from_integer(integer_chara_id))
        {
            _memories[*id].kill_count = legacy_npcmemory(0, i);
            _memories[*id].generate_count = legacy_npcmemory(1, i);
        }
    }
}



void ItemMemoryTable::pack_to(elona_vector2<int>& legacy_itemmemory) const
{
    for (int i = 0; i < 800; ++i)
    {
        const auto integer_item_id = i;
        if (const auto id = the_item_db.get_id_from_integer(integer_item_id))
        {
            if (const auto itr = _memories.find(*id); itr != _memories.end())
            {
                legacy_itemmemory(0, i) =
                    static_cast<int>(itr->second.identify_state);
                legacy_itemmemory(1, i) = itr->second.generate_count;
                legacy_itemmemory(2, i) = itr->second._is_reserved
                    ? 2
                    : (itr->second.is_decoded ? 1 : 0);
            }
        }
    }
}



void ItemMemoryTable::unpack_from(elona_vector2<int>& legacy_itemmemory)
{
    _memories.clear();

    for (int i = 0; i < 800; ++i)
    {
        if (legacy_itemmemory(0, i) == 0 && legacy_itemmemory(1, i) == 0 &&
            legacy_itemmemory(2, i) == 0)
            continue;

        const auto integer_item_id = i;
        if (const auto id = the_item_db.get_id_from_integer(integer_item_id))
        {
            _memories[*id].identify_state =
                static_cast<IdentifyState>(legacy_itemmemory(0, i));
            _memories[*id].generate_count = legacy_itemmemory(1, i);
            _memories[*id].is_decoded = legacy_itemmemory(2, i) != 0;
            _memories[*id]._is_reserved = legacy_itemmemory(2, i) == 2;
        }
    }
}



void CraftingMaterialBag::pack_to(elona_vector1<int>& legacy_mat) const
{
    for (int i = 0; i < 400; ++i)
    {
        const auto integer_material_id = i;
        if (const auto id = the_crafting_material_db.get_id_from_integer(
                integer_material_id))
        {
            if (const auto itr = _materials.find(*id); itr != _materials.end())
            {
                legacy_mat(i) = itr->second;
            }
        }
    }
}



void CraftingMaterialBag::unpack_from(elona_vector1<int>& legacy_mat)
{
    _materials.clear();

    for (int i = 0; i < 400; ++i)
    {
        if (legacy_mat(i) == 0)
            continue;

        const auto integer_material_id = i;
        if (const auto id = the_crafting_material_db.get_id_from_integer(
                integer_material_id))
        {
            _materials[*id] = legacy_mat(i);
        }
    }
}

} // namespace elona
