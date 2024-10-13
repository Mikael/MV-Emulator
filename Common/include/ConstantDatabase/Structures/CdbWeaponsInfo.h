#ifndef CDB_WEAPON_INFO_H
#define CDB_WEAPON_INFO_H

#include "visit_struct/visit_struct.hpp"
#include <nlohmann/json.hpp>

namespace Common
{
    namespace ConstantDatabase
    {
#pragma pack(push, 1) 
        struct CdbWeaponInfo
        {
            int ii_id;
            char ii_name[50];
            char ii_name_option[50];
            char ii_name_time[50];
            int ii_type;
            int ii_type_inven;
            bool ii_inven_usable;
            int ii_type_pcbang;
            int ii_package_result;
            bool ii_dress;
            bool ii_hide_hair;
            bool ii_hide_face;
            bool ii_hide_back;
            bool ii_class_a;
            bool ii_class_b;
            bool ii_class_c;
            bool ii_class_d;
            bool ii_class_e;
            bool ii_class_f;
            bool ii_class_g;
            bool ii_class_h;
            bool ii_class_i;
            bool ii_class_j;
            bool ii_class_k;
            bool ii_class_l;
            bool ii_class_m;
            bool ii_class_n;
            bool ii_class_o;
            bool ii_class_p;
            int ii_grade;
            int ii_stocks;
            bool ii_usable;
            bool ii_upgradable;
            bool ii_consumable;
            int ii_weaponinfo;
            int ii_fullseteffectinfo;
            int ii_durable_value;
            int ii_durable_factor;
            int ii_durable_repair_type;
            bool ii_limited_mod;
            int ii_limited_time;
            int ii_buy_coupon;
            int ii_buy_cash;
            int ii_buy_point;
            int ii_sell_point;
            int ii_bonus_point;
            int ii_bonus_pack;
            int ii_dioramainfo;
            int ii_dummyinfo;
            int ii_icon;
            int ii_iconsmall;
            char ii_meshfilename[50];
            char ii_nodename[50];
            char ii_color_ambient[50];
            char ii_color_diffuse[50];
            char ii_color_specular[50];
            char ii_color_emittance[50];
            int ii_sfx;
            int ef_effect_1;
            int ef_target_1;
            int ef_effect_2;
            int ef_target_2;
            int ef_effect_3;
            int ef_target_3;
            char ii_desc[200];
            bool ii_immediately_set;
            bool ii_is_trade;
            int ii_ei_exp;
            bool ii_evolution_type;

            // Method to convert to JSON
            nlohmann::json toJson() const
            {
                nlohmann::json jsonObject;
                jsonObject["id"] = ii_id;
                jsonObject["name"] = std::string(ii_name); // Convert to std::string
                jsonObject["name_option"] = std::string(ii_name_option);
                jsonObject["name_time"] = std::string(ii_name_time);
                jsonObject["type"] = ii_type;
                jsonObject["type_inven"] = ii_type_inven;
                jsonObject["inven_usable"] = ii_inven_usable;
                jsonObject["type_pcbang"] = ii_type_pcbang;
                jsonObject["package_result"] = ii_package_result;
                jsonObject["dress"] = ii_dress;
                jsonObject["hide_hair"] = ii_hide_hair;
                jsonObject["hide_face"] = ii_hide_face;
                jsonObject["hide_back"] = ii_hide_back;
                jsonObject["class_a"] = ii_class_a;
                jsonObject["class_b"] = ii_class_b;
                jsonObject["class_c"] = ii_class_c;
                jsonObject["class_d"] = ii_class_d;
                jsonObject["class_e"] = ii_class_e;
                jsonObject["class_f"] = ii_class_f;
                jsonObject["class_g"] = ii_class_g;
                jsonObject["class_h"] = ii_class_h;
                jsonObject["class_i"] = ii_class_i;
                jsonObject["class_j"] = ii_class_j;
                jsonObject["class_k"] = ii_class_k;
                jsonObject["class_l"] = ii_class_l;
                jsonObject["class_m"] = ii_class_m;
                jsonObject["class_n"] = ii_class_n;
                jsonObject["class_o"] = ii_class_o;
                jsonObject["class_p"] = ii_class_p;
                jsonObject["grade"] = ii_grade;
                jsonObject["stocks"] = ii_stocks;
                jsonObject["usable"] = ii_usable;
                jsonObject["upgradable"] = ii_upgradable;
                jsonObject["consumable"] = ii_consumable;
                jsonObject["weaponinfo"] = ii_weaponinfo;
                jsonObject["fullseteffectinfo"] = ii_fullseteffectinfo;
                jsonObject["durable_value"] = ii_durable_value;
                jsonObject["durable_factor"] = ii_durable_factor;
                jsonObject["durable_repair_type"] = ii_durable_repair_type;
                jsonObject["limited_mod"] = ii_limited_mod;
                jsonObject["limited_time"] = ii_limited_time;
                jsonObject["buy_coupon"] = ii_buy_coupon;
                jsonObject["buy_cash"] = ii_buy_cash;
                jsonObject["buy_point"] = ii_buy_point;
                jsonObject["sell_point"] = ii_sell_point;
                jsonObject["bonus_point"] = ii_bonus_point;
                jsonObject["bonus_pack"] = ii_bonus_pack;
                jsonObject["dioramainfo"] = ii_dioramainfo;
                jsonObject["dummyinfo"] = ii_dummyinfo;
                jsonObject["icon"] = ii_icon;
                jsonObject["iconsmall"] = ii_iconsmall;
                jsonObject["meshfilename"] = std::string(ii_meshfilename);
                jsonObject["nodename"] = std::string(ii_nodename);
                jsonObject["color_ambient"] = std::string(ii_color_ambient);
                jsonObject["color_diffuse"] = std::string(ii_color_diffuse);
                jsonObject["color_specular"] = std::string(ii_color_specular);
                jsonObject["color_emittance"] = std::string(ii_color_emittance);
                jsonObject["sfx"] = ii_sfx;
                jsonObject["ef_effect_1"] = ef_effect_1;
                jsonObject["ef_target_1"] = ef_target_1;
                jsonObject["ef_effect_2"] = ef_effect_2;
                jsonObject["ef_target_2"] = ef_target_2;
                jsonObject["ef_effect_3"] = ef_effect_3;
                jsonObject["ef_target_3"] = ef_target_3;
                jsonObject["desc"] = std::string(ii_desc);
                jsonObject["immediately_set"] = ii_immediately_set;
                jsonObject["is_trade"] = ii_is_trade;
                jsonObject["ei_exp"] = ii_ei_exp;
                jsonObject["evolution_type"] = ii_evolution_type;

                return jsonObject;
            }
        };

#pragma pack(pop)
    }
}

VISITABLE_STRUCT(Common::ConstantDatabase::CdbWeaponInfo, ii_id, ii_name, ii_name_option, ii_name_time, ii_type, ii_type_inven, ii_inven_usable, ii_type_pcbang, ii_package_result, ii_dress, ii_hide_hair, ii_hide_face, ii_hide_back, ii_class_a, ii_class_b,
    ii_class_c, ii_class_d, ii_class_e, ii_class_f, ii_class_g, ii_class_h, ii_class_i, ii_class_j, ii_class_k, ii_class_l, ii_class_m, ii_class_n, ii_class_o, ii_class_p, ii_grade, ii_stocks, ii_usable,
    ii_upgradable, ii_consumable, ii_weaponinfo, ii_fullseteffectinfo, ii_durable_value, ii_durable_factor, ii_durable_repair_type, ii_limited_mod, ii_limited_time, ii_buy_coupon, ii_buy_cash, ii_buy_point,
    ii_sell_point, ii_bonus_point, ii_bonus_pack, ii_dioramainfo, ii_dummyinfo, ii_icon, ii_iconsmall, ii_meshfilename, ii_nodename, ii_color_ambient, ii_color_diffuse, ii_color_specular, ii_color_emittance,
    ii_sfx, ef_effect_1, ef_target_1, ef_effect_2, ef_target_2, ef_effect_3, ef_target_3, ii_desc, ii_immediately_set, ii_is_trade, ii_ei_exp, ii_evolution_type
);

#endif
