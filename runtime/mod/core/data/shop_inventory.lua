local data = { _table = {} }
function data:add_multi(tbl)
   for _, v in ipairs(tbl) do
      self:add(v)
   end
end
function data:add(data)
   self._table[data.id] = data
end



local Rand = Elona.require("Rand")
local Item = Elona.require("Item")
local Math = Elona.require("Math")
local World = Elona.require("World")

-- Generates a list to be used with "choices" which will set the
-- provided field to one of the choices in "list".
local function make_choices_list(list, field_name)
   for i, v in ipairs(list) do
      list[i] = { index = i - 1 }
      list[i][field_name] = v
   end
   return list
end

local function make_id_list(list)
   return make_choices_list(list, "id")
end

local function make_filter_list(list)
   return make_choices_list(list, "flttypemajor")
end

local deed_items = make_id_list(
   {
      "core.deed",
      "core.deed_of_museum",
      "core.deed_of_shop",
      "core.deed_of_farm",
      "core.deed_of_storage_house",
      "core.shelter",
      "core.deed_of_ranch"
   }
)

local medal_items = make_id_list(
   {
      "core.scroll_of_growth",
      "core.scroll_of_faith",
      "core.scroll_of_superior_material",
      "core.rod_of_domination",
      "core.artifact_seed",
      "core.presidents_chair",
      "core.bill",
      "core.potion_of_cure_corruption",
      "core.bottle_of_water",
      "core.tax_masters_tax_box",
      "core.cat_sisters_diary",
      "core.little_sisters_diary",
      "core.girls_diary",
      "core.shrine_gate",
      "core.bottle_of_hermes_blood",
      "core.sages_helm",
      "core.diablo",
      "core.license_of_the_void_explorer",
      "core.garoks_hammer"
   }
)

-- NOTE: duplicated from fsetwear.
local filter_set_wear = make_filter_list({10000, 10000, 24000, 24000,
                                          25000, 12000, 16000, 20000,
                                          19000, 18000, 22000, 14000,
                                          32000, 34000})

local merchant_rules = {
   { choices = filter_set_wear },
   { fixlv = "Great" },
   { one_in = 2, fixlv = "Miracle" },
}

local function merchant_item_number()
   return 4 + Rand.rnd(4)
end

--[[
Shop inventory generation is defined using the "rules" field, which is
an array of rules to apply to a generated item. Each rule consists of
one or zero predicates and some properties used in the generation of
the item. This allows inserting/removing values or altering
probabilities without needing to create a new generation function
every time.

The properties that end up being generated by the ruleset will be sent
to Item.create as the arguments table, so whichever arguments
Item.create supports are supported as properties here. Any
unrecognized properties are ignored.

Some notes:
  - Each rule is applied in order of definition in the "rules"
    list.
  - Only the first predicate found in each rule will be applied. If
    no predicates are found, the rule is always applied.

Rule predicates:
  index = 2
    the property is always applied when the 2nd item is being
    generated. Use for defining a set array of items to create.
  one_in = 5
    the property is applied one out of every 5 times. Equivalent to
    rnd(5) == 0.
  all_but_one_in = 5
    the property is always applied except one out of every 5 times.
    Equivalent to rnd(5) != 0.
  predicate = function(args) return args.index > 10 end
    a function, to which the following arguments are passed as a
    table. If it returns true, the properties are applied.
      index: index of the item being generated.
      shopkeeper: character who is the shopkeeper.

Available properties:
  id: string id of the item. If it equals "Skip", skip generation
      of an item this cycle.
  flttypeminor: number.
  flttypemajor: number.
  fixlv: string enum indicating quality.
  fltn: used with fltn().
  choices: an array of properties. one out of the set of properties
           provided will be applied.
  on_generate: a function that will have a table with these fields
               passed to it.
    index: index of the item being generated.
    shopkeeper: character who is the shopkeeper.
]]

-- NOTE: "id" must be the same as a character role, and between
-- [1000,1999]. (2003 is special-cased.)
data:add_multi(
   {
      {
         id = "magic_vendor",
         legacy_id = 1004,
         rules = {
            {
               choices = {
                  {flttypeminor = 52001},
                  {flttypemajor = 53000},
                  {flttypemajor = 52000},
               }
            },
            { one_in = 7, flttypemajor = 54000 },
            { one_in = 15, flttypemajor = 55000 },
            { one_in = 20, id = "core.recipe" },
         }
      },
      {
         id = "younger_sister_of_mansion",
         legacy_id = 1019,
         rules = {
            { id = "core.sisters_love_fueled_lunch" }
         }
      },
      {
         id = "spell_writer",
         legacy_id = 1020,
         rules = {
            {
               on_generate = function()
                  local reserved = {}
                  for item_id, _ in pairs(data.raw["core.item"]) do
                     if Item.memory(2, item_id) > 1 then
                        reserved[#reserved+1] = item_id
                     end
                  end

                  if #reserved == 0 then
                     -- NOTE: this used to return out of shop_refresh,
                     -- skipping the update of time_to_restore.
                     -- However, it would be strange to have no books
                     -- reserved, then reserve one and suddenly see
                     -- them available by talking to the shopkeeper
                     -- again immediately, so now the behavior is to
                     -- update time_to_restore anyway if no books are
                     -- reserved at the time of refresh.
                     return { id = "Skip" }
                  end

                  return { id = Rand.choice(reserved) }
               end
            }
         },
         item_base_value = function(args)
            return args.item.value * 3 / 2
         end
      },
      {
         id = "moyer",
         legacy_id = 1015,
         rules = {
            {
               choices = {
                  {flttypemajor = 59000},
                  {flttypemajor = 32000},
                  {flttypemajor = 34000},
               }
            },
            { one_in = 3, fixlv = "Great" },
            { one_in = 10, fixlv = "Miracle" },
         },
         item_base_value = function(args)
            return args.item.value * 2
         end
      },
      {
         id = "general_vendor",
         legacy_id = 1006,
         rules = {
            {
               choices = {
                  {flttypemajor = 25000},
                  {flttypemajor = 60000},
                  {flttypemajor = 20000},
                  {flttypemajor = 77000},
                  {flttypemajor = 59000},
               }
            },
            { one_in = 20, id = "core.small_gamble_chest" },
            { one_in = 8, flttypemajor = 91000 },
            { one_in = 10, choices = deed_items },
         }
      },
      {
         id = "bakery",
         legacy_id = 1003,
         rules = {
            { all_but_one_in = 3, id = "Skip" },
            {
               choices = {
                  {flttypeminor = 57001},
                  {flttypeminor = 57001},
                  {flttypeminor = 57002},
               }
            },
         }
      },
      {
         id = "food_vendor",
         legacy_id = 1002,
         rules = {
            { all_but_one_in = 3, id = "Skip" },
            { flttypemajor = 57000 },
            { one_in = 5, flttypemajor = 91000 },
         }
      },
      {
         id = "blackmarket",
         legacy_id = 1007,
         rules = {
            { choices = filter_set_wear },
            { one_in = 3, fixlv = "Great" },
            { one_in = 10, fixlv = "Miracle" },
         },
         item_number = function(args)
            return 6 + args.shopkeeper.shop_rank / 10
         end,
         item_base_value = function(args)
            if World.belongs_to_guild("thieves") then
               return args.item.value * 2
            else
               return args.item.value * 3
            end
         end
      },
      {
         id = "wandering_merchant",
         legacy_id = 1010,
         rules = merchant_rules,
         item_number = merchant_item_number,
         item_base_value = function(args)
            return args.item.value * 2
         end,
         is_temporary = true -- Uses shop ID 1.
      },
      {
         id = "visiting_merchant",
         -- NOTE: the only shop ID for which (id / 1000) != 1.
         legacy_id = 2003,
         rules = merchant_rules,
         item_number = merchant_item_number,
         item_base_value = function(args)
            return args.item.value * 4 / 5
         end,
         is_temporary = true -- Uses shop ID 1.
      },
      {
         id = "innkeeper",
         legacy_id = 1005,
         rules = {
            { flttypemajor = 91000 },
            { one_in = 4, flttypeminor = 52002 },
            { one_in = 20, id = "core.small_gamble_chest" },
         }
      },
      {
         id = "general_store",
         legacy_id = 1008,
         rules = {
            { flttypemajor = 56000 },
            { one_in = 3, choices = filter_set_wear },
            { one_in = 3, flttypemajor = 60000 },
            { one_in = 5, flttypemajor = 57000 },
            { one_in = 4, flttypemajor = 53000 },
            { one_in = 15, flttypemajor = 55000 },
            { one_in = 10, flttypemajor = 91000 },
            { one_in = 10, choices = deed_items },
            { one_in = 15, id = "core.deed_of_heirship" },
         }
      },
      {
         id = "blacksmith",
         legacy_id = 1001,
         rules = {
            {
               choices = {
                  {flttypemajor = 16000},
                  {flttypemajor = 12000},
                  {flttypemajor = 22000},
                  {flttypemajor = 18000},
                  {flttypemajor = 14000},
                  {flttypemajor = 19000},
               }
            },
            {
               one_in = 3,
               choices = {
                  {flttypemajor = 10000},
                  {flttypemajor = 24000},
                  {flttypemajor = 24000},
               }
            }
         }
      },
      {
         -- NOTE: Has these special-case behaviors.
         --  + Extra filtering for cargo items when buying/selling
         --    through the "shoptrade" flag.
         --  + You can always sell cargo to traders regardless of how
         --    much money the trader has on hand.
         --  + On shop refresh, updates the buying rates of each cargo
         --    type based on the current map.
         id = "trader",
         legacy_id = 1009,
         rules = {
            { flttypemajor = 92000 },
         }
      },
      {
         id = "the_fence",
         legacy_id = 1021,
         rules = {
            { flttypemajor = 59000 },
            { one_in = 2, id = "core.lockpick" },
            { one_in = 2, id = "core.disguise_set" },
         }
      },
      {
         id = "sales_person_a",
         legacy_id = 1011,
         rules = {
            { one_in = 4, flttypemajor = 24000 },
            { one_in = 5, flttypemajor = 24000 },
            { one_in = 3, flttypemajor = 57000 },
            { fltn = "sf" },
         }
      },
      {
         id = "sales_person_c",
         legacy_id = 1013,
         rules = {
            { all_but_one_in = 3, id = "Skip" },
            { flttypemajor = 55000 },
            { one_in = 3, choices = deed_items },
            { one_in = 5, id = "core.deed_of_heirship" },
         }
      },
      {
         id = "souvenir_vendor",
         legacy_id = 1018,
         ignores_noshop = true,
         rules = {
            { fltn = "spshop" },
         },
         item_number = function(args) return args.item_number / 2 end,
         item_base_value = function(args)
            local price = Math.clamp(args.item.value, 1, 1000000) * 50
            if args.item.id == "core.gift" then
               price = price * 10
            end
            return price
         end
      },
      {
         id = "street_vendor",
         legacy_id = 1022,
         rules = {
            { fltn = "fest" },
            { one_in = 12, id = "core.upstairs" },
            { one_in = 12, id = "core.downstairs" },
            { one_in = 5, id = "core.bottle_of_soda" },
            { one_in = 12, id = "core.festival_wreath" },
            { one_in = 12, id = "core.new_years_decoration" },
            { one_in = 12, id = "core.miniature_tree" },
         }
      },
      {
         id = "dye_vendor",
         legacy_id = 1017,
         rules = {
            { id = "core.bottle_of_dye" },
         }
      },
      {
         id = "sales_person_b",
         legacy_id = 1012,
         rules = {
            { flttypemajor = 60000 },
            { index = 0, id = "core.microwave_oven" },
            { index = 1, id = "core.shop_strongbox" },
            { index = 2, id = "core.register" },
            { index = 3, id = "core.salary_chest" },
            { index = 4, id = "core.freezer" },
            { index = 5, id = "core.playback_disc" },
            { index = 6, id = "core.house_board" },
            { predicate = function(args) return args.index > 10 and not Rand.one_in(3) end, id = "Skip" },
            { index = 19, id = "core.red_treasure_machine" },
            { index = 20, id = "core.blue_treasure_machine" },
            { index = 21, id = "core.tax_masters_tax_box" },
         },
      },
      {
         id = "fisher",
         legacy_id = 1014,
         rules = {
            { id = "core.bait" }
         }
      },
      {
         -- NOTE: Has these special-case behaviors.
         --  + Normal generation behavior of sold item number/curse
         --    state is replaced with on_generate_item below. (the
         --    presence of on_generate_item causes all the generation
         --    behavior done after the item is created with
         --    Item.create to be skipped.)
         --  + Item base value is ignored and ctrl_inventory()
         --    (currently) uses a hardcoded value for the number of
         --    medals to sell at.
         --  + Items with Special quality or the precious flag set are
         --    permitted to be sold through a special inventory
         --    routine type in ctrl_inventory() which can only be
         --    triggered through Miral's dialog. In normal shops,
         --    items with those properties are not displayed even if
         --    they are generated successfully.
         id = "miral",
         legacy_id = 1016,
         rules = medal_items,
         item_number = function() return #medal_items end,
         on_generate_item = function(args)
            args.item.number = 1
            args.item.curse_state = "None"
            if args.item.id == "core.rod_of_domination" then
               args.item.count = 4
            end
         end
      }
})



return { ["core.shop_inventory"] = data._table }