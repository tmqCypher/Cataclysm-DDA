#include "map_extras.h"

#include <algorithm>
#include <array>
#include <cstdlib>
#include <functional>
#include <map>
#include <memory>
#include <new>
#include <set>
#include <unordered_map>
#include <utility>
#include <vector>

#include "auto_note.h"
#include "calendar.h"
#include "cata_utility.h"
#include "cellular_automata.h"
#include "character_id.h"
#include "colony.h"
#include "coordinate_conversions.h"
#include "coordinates.h"
#include "creature_tracker.h"
#include "debug.h"
#include "enum_conversions.h"
#include "enums.h"
#include "field_type.h"
#include "fungal_effects.h"
#include "game_constants.h"
#include "generic_factory.h"
#include "item.h"
#include "item_group.h"
#include "json.h"
#include "line.h"
#include "map.h"
#include "map_iterator.h"
#include "mapdata.h"
#include "mapgen.h"
#include "mapgen_functions.h"
#include "mapgendata.h"
#include "mongroup.h"
#include "optional.h"
#include "options.h"
#include "overmap.h"
#include "overmapbuffer.h"
#include "point.h"
#include "regional_settings.h"
#include "rng.h"
#include "string_formatter.h"
#include "string_id.h"
#include "text_snippets.h"
#include "translations.h"
#include "trap.h"
#include "type_id.h"
#include "ui.h"
#include "units.h"
#include "veh_type.h"
#include "vehicle.h"
#include "vehicle_group.h"
#include "vpart_position.h"
#include "vpart_range.h"
#include "weighted_list.h"

static const flag_id json_flag_FILTHY( "FILTHY" );

static const furn_str_id furn_f_sign_warning( "f_sign_warning" );

static const item_group_id Item_spawn_data_ammo_casings( "ammo_casings" );
static const item_group_id Item_spawn_data_army_bed( "army_bed" );
static const item_group_id Item_spawn_data_everyday_corpse( "everyday_corpse" );
static const item_group_id Item_spawn_data_grenades( "grenades" );
static const item_group_id Item_spawn_data_guns_rifle_milspec( "guns_rifle_milspec" );
static const item_group_id Item_spawn_data_map_extra_casings( "map_extra_casings" );
static const item_group_id Item_spawn_data_map_extra_police( "map_extra_police" );
static const item_group_id Item_spawn_data_mil_armor( "mil_armor" );
static const item_group_id Item_spawn_data_mil_bulk( "mil_bulk" );
static const item_group_id Item_spawn_data_mil_food( "mil_food" );
static const item_group_id
Item_spawn_data_military_standard_assault_rifles( "military_standard_assault_rifles" );
static const item_group_id Item_spawn_data_military_standard_lmgs( "military_standard_lmgs" );
static const item_group_id
Item_spawn_data_military_standard_shotguns( "military_standard_shotguns" );
static const item_group_id
Item_spawn_data_military_standard_sniper_rifles( "military_standard_sniper_rifles" );
static const item_group_id Item_spawn_data_mine_equipment( "mine_equipment" );
static const item_group_id
Item_spawn_data_mon_zombie_soldier_death_drops( "mon_zombie_soldier_death_drops" );
static const item_group_id Item_spawn_data_rare( "rare" );
static const item_group_id Item_spawn_data_remains_human_generic( "remains_human_generic" );
static const item_group_id Item_spawn_data_trash_cart( "trash_cart" );

static const itype_id itype_223_casing( "223_casing" );
static const itype_id itype_762_51_casing( "762_51_casing" );
static const itype_id itype_9mm_casing( "9mm_casing" );
static const itype_id itype_acoustic_guitar( "acoustic_guitar" );
static const itype_id itype_ash( "ash" );
static const itype_id itype_bag_canvas( "bag_canvas" );
static const itype_id itype_bottle_glass( "bottle_glass" );
static const itype_id itype_chunk_sulfur( "chunk_sulfur" );
static const itype_id itype_hatchet( "hatchet" );
static const itype_id itype_landmine( "landmine" );
static const itype_id itype_material_sand( "material_sand" );
static const itype_id itype_material_soil( "material_soil" );
static const itype_id itype_rag( "rag" );
static const itype_id itype_shot_hull( "shot_hull" );
static const itype_id itype_splinter( "splinter" );
static const itype_id itype_stanag30( "stanag30" );
static const itype_id itype_stick( "stick" );
static const itype_id itype_stick_long( "stick_long" );
static const itype_id itype_tire_iron( "tire_iron" );
static const itype_id itype_vodka( "vodka" );
static const itype_id itype_wheel( "wheel" );
static const itype_id itype_withered( "withered" );

static const map_extra_id map_extra_mx_bandits_block( "mx_bandits_block" );
static const map_extra_id map_extra_mx_burned_ground( "mx_burned_ground" );
static const map_extra_id map_extra_mx_casings( "mx_casings" );
static const map_extra_id map_extra_mx_city_trap( "mx_city_trap" );
static const map_extra_id map_extra_mx_clay_deposit( "mx_clay_deposit" );
static const map_extra_id map_extra_mx_clearcut( "mx_clearcut" );
static const map_extra_id map_extra_mx_corpses( "mx_corpses" );
static const map_extra_id map_extra_mx_crater( "mx_crater" );
static const map_extra_id map_extra_mx_dead_vegetation( "mx_dead_vegetation" );
static const map_extra_id map_extra_mx_grove( "mx_grove" );
static const map_extra_id map_extra_mx_helicopter( "mx_helicopter" );
static const map_extra_id map_extra_mx_house_spider( "mx_house_spider" );
static const map_extra_id map_extra_mx_house_wasp( "mx_house_wasp" );
static const map_extra_id map_extra_mx_jabberwock( "mx_jabberwock" );
static const map_extra_id map_extra_mx_looters( "mx_looters" );
static const map_extra_id map_extra_mx_mayhem( "mx_mayhem" );
static const map_extra_id map_extra_mx_military( "mx_military" );
static const map_extra_id map_extra_mx_minefield( "mx_minefield" );
static const map_extra_id map_extra_mx_null( "mx_null" );
static const map_extra_id map_extra_mx_point_burned_ground( "mx_point_burned_ground" );
static const map_extra_id map_extra_mx_point_dead_vegetation( "mx_point_dead_vegetation" );
static const map_extra_id map_extra_mx_pond( "mx_pond" );
static const map_extra_id map_extra_mx_portal_in( "mx_portal_in" );
static const map_extra_id map_extra_mx_reed( "mx_reed" );
static const map_extra_id map_extra_mx_roadblock( "mx_roadblock" );
static const map_extra_id map_extra_mx_roadworks( "mx_roadworks" );
static const map_extra_id map_extra_mx_shia( "mx_shia" );
static const map_extra_id map_extra_mx_shrubbery( "mx_shrubbery" );
static const map_extra_id map_extra_mx_supplydrop( "mx_supplydrop" );

static const mongroup_id GROUP_FISH( "GROUP_FISH" );
static const mongroup_id GROUP_FUNGI_FUNGALOID( "GROUP_FUNGI_FUNGALOID" );
static const mongroup_id GROUP_MAYBE_MIL( "GROUP_MAYBE_MIL" );
static const mongroup_id GROUP_NETHER_CAPTURED( "GROUP_NETHER_CAPTURED" );
static const mongroup_id GROUP_NETHER_PORTAL( "GROUP_NETHER_PORTAL" );
static const mongroup_id GROUP_STRAY_DOGS( "GROUP_STRAY_DOGS" );
static const mongroup_id GROUP_WASP_GUARD( "GROUP_WASP_GUARD" );
static const mongroup_id GROUP_WASP_QUEEN( "GROUP_WASP_QUEEN" );

static const mtype_id mon_dermatik( "mon_dermatik" );
static const mtype_id mon_dispatch( "mon_dispatch" );
static const mtype_id mon_jabberwock( "mon_jabberwock" );
static const mtype_id mon_shia( "mon_shia" );
static const mtype_id mon_spider_cellar_giant( "mon_spider_cellar_giant" );
static const mtype_id mon_spider_widow_giant( "mon_spider_widow_giant" );
static const mtype_id mon_turret_riot( "mon_turret_riot" );
static const mtype_id mon_turret_searchlight( "mon_turret_searchlight" );
static const mtype_id mon_turret_speaker( "mon_turret_speaker" );
static const mtype_id mon_wolf( "mon_wolf" );
static const mtype_id mon_zombie_bio_op( "mon_zombie_bio_op" );
static const mtype_id mon_zombie_military_pilot( "mon_zombie_military_pilot" );
static const mtype_id mon_zombie_scientist( "mon_zombie_scientist" );
static const mtype_id mon_zombie_soldier( "mon_zombie_soldier" );

static const oter_type_str_id oter_type_bridge( "bridge" );
static const oter_type_str_id oter_type_bridgehead_ground( "bridgehead_ground" );
static const oter_type_str_id oter_type_road( "road" );

static const relic_procgen_id relic_procgen_data_alien_reality( "alien_reality" );

static const string_id<class npc_template> npc_template_bandit( "bandit" );

static const ter_str_id ter_t_dirt( "t_dirt" );
static const ter_str_id ter_t_grass_dead( "t_grass_dead" );
static const ter_str_id ter_t_stump( "t_stump" );
static const ter_str_id ter_t_tree_birch_harvested( "t_tree_birch_harvested" );
static const ter_str_id ter_t_tree_dead( "t_tree_dead" );
static const ter_str_id ter_t_tree_deadpine( "t_tree_deadpine" );
static const ter_str_id ter_t_tree_hickory_dead( "t_tree_hickory_dead" );
static const ter_str_id ter_t_trunk( "t_trunk" );

static const trap_str_id tr_caltrops( "tr_caltrops" );
static const trap_str_id tr_engine( "tr_engine" );
static const trap_str_id tr_nailboard( "tr_nailboard" );

static const vgroup_id VehicleGroup_crashed_helicopters( "crashed_helicopters" );
static const vgroup_id VehicleGroup_military_vehicles( "military_vehicles" );

static const vproto_id vehicle_prototype_4x4_car( "4x4_car" );
static const vproto_id vehicle_prototype_car( "car" );
static const vproto_id vehicle_prototype_car_fbi( "car_fbi" );
static const vproto_id vehicle_prototype_excavator( "excavator" );
static const vproto_id vehicle_prototype_humvee( "humvee" );
static const vproto_id vehicle_prototype_limousine( "limousine" );
static const vproto_id vehicle_prototype_military_cargo_truck( "military_cargo_truck" );
static const vproto_id vehicle_prototype_policecar( "policecar" );
static const vproto_id vehicle_prototype_road_roller( "road_roller" );

class npc_template;

namespace io
{

template<>
std::string enum_to_string<map_extra_method>( map_extra_method data )
{
    switch( data ) {
        // *INDENT-OFF*
        case map_extra_method::null: return "null";
        case map_extra_method::map_extra_function: return "map_extra_function";
        case map_extra_method::mapgen: return "mapgen";
        case map_extra_method::update_mapgen: return "update_mapgen";
        case map_extra_method::num_map_extra_methods: break;
        // *INDENT-ON*
    }
    cata_fatal( "Invalid map_extra_method" );
}

} // namespace io

namespace
{

generic_factory<map_extra> extras( "map extra" );

} // namespace

/** @relates string_id */
template<>
const map_extra &string_id<map_extra>::obj() const
{
    return extras.obj( *this );
}

/** @relates string_id */
template<>
bool string_id<map_extra>::is_valid() const
{
    return extras.is_valid( *this );
}

namespace MapExtras
{

const generic_factory<map_extra> &mapExtraFactory()
{
    return extras;
}

void clear()
{
    extras.reset();
}

static bool mx_null( map &, const tripoint & )
{
    debugmsg( "Tried to generate null map extra." );

    return false;
}

static void dead_vegetation_parser( map &m, const tripoint &loc )
{
    // furniture plants die to withered plants
    const furn_t &fid = m.furn( loc ).obj();
    if( fid.has_flag( ter_furn_flag::TFLAG_PLANT ) || fid.has_flag( ter_furn_flag::TFLAG_FLOWER ) ||
        fid.has_flag( ter_furn_flag::TFLAG_ORGANIC ) ) {
        m.i_clear( loc );
        m.furn_set( loc, f_null );
        m.spawn_item( loc, itype_withered );
    }
    // terrain specific conversions
    const ter_id tid = m.ter( loc );
    static const std::map<ter_id, ter_str_id> dies_into {{
            {t_grass, ter_t_grass_dead},
            {t_grass_long, ter_t_grass_dead},
            {t_grass_tall, ter_t_grass_dead},
            {t_moss, ter_t_grass_dead},
            {t_tree_pine, ter_t_tree_deadpine},
            {t_tree_birch, ter_t_tree_birch_harvested},
            {t_tree_willow, ter_t_tree_dead},
            {t_tree_hickory, ter_t_tree_hickory_dead},
            {t_tree_hickory_harvested, ter_t_tree_hickory_dead},
            {t_grass_golf, ter_t_grass_dead},
            {t_grass_white, ter_t_grass_dead},
        }};

    const auto iter = dies_into.find( tid );
    if( iter != dies_into.end() ) {
        m.ter_set( loc, iter->second );
    }
    // non-specific small vegetation falls into sticks, large dies and randomly falls
    const ter_t &tr = tid.obj();
    if( tr.has_flag( ter_furn_flag::TFLAG_SHRUB ) ) {
        m.ter_set( loc, t_dirt );
        if( one_in( 2 ) ) {
            m.spawn_item( loc, itype_stick );
        }
    } else if( tr.has_flag( ter_furn_flag::TFLAG_TREE ) ) {
        if( one_in( 4 ) ) {
            m.ter_set( loc, ter_t_trunk );
        } else if( one_in( 4 ) ) {
            m.ter_set( loc, ter_t_stump );
        } else {
            m.ter_set( loc, ter_t_tree_dead );
        }
    } else if( tr.has_flag( ter_furn_flag::TFLAG_YOUNG ) ) {
        m.ter_set( loc, ter_t_dirt );
        if( one_in( 2 ) ) {
            m.spawn_item( loc, itype_stick_long );
        }
    }
}

static bool mx_house_wasp( map &m, const tripoint &loc )
{
    for( int i = 0; i < SEEX * 2; i++ ) {
        for( int j = 0; j < SEEY * 2; j++ ) {
            if( m.ter( point( i, j ) ) == t_door_c || m.ter( point( i, j ) ) == t_door_locked ) {
                m.ter_set( point( i, j ), t_door_frame );
            }
            if( m.ter( point( i, j ) ) == t_window_domestic && !one_in( 3 ) ) {
                m.ter_set( point( i, j ), t_window_frame );
            }
            if( m.ter( point( i, j ) ) == t_wall && one_in( 8 ) ) {
                m.ter_set( point( i, j ), t_paper );
            }
        }
    }
    const int num_pods = rng( 8, 12 );
    for( int i = 0; i < num_pods; i++ ) {
        const point pod( rng( 1, SEEX * 2 - 2 ), rng( 1, SEEY * 2 - 2 ) );
        point non;
        while( non.x == 0 && non.y == 0 ) {
            non.x = rng( -1, 1 );
            non.y = rng( -1, 1 );
        }
        for( int x = -1; x <= 1; x++ ) {
            for( int y = -1; y <= 1; y++ ) {
                if( ( x != non.x || y != non.y ) && ( x != 0 || y != 0 ) ) {
                    m.ter_set( pod + point( x, y ), t_paper );
                }
            }
        }
        m.place_spawns( GROUP_WASP_GUARD, 1, pod, pod, 1, true );
    }
    m.place_spawns( GROUP_WASP_QUEEN, 1, point_zero, point( SEEX, SEEY ), 1, true );
    if( one_in( 5 ) ) {
        m.add_spawn( mon_dermatik, rng( 1, 3 ), tripoint( point( SEEX * 2 - 1, SEEY * 2 - 1 ), loc.z ) );
    }

    return true;
}

static bool mx_house_spider( map &m, const tripoint &loc )
{
    auto spider_type = mon_spider_widow_giant;
    auto egg_type = f_egg_sackbw;
    if( one_in( 2 ) ) {
        spider_type = mon_spider_cellar_giant;
        egg_type = f_egg_sackcs;
    }
    for( int i = 0; i < SEEX * 2; i++ ) {
        for( int j = 0; j < SEEY * 2; j++ ) {
            if( m.ter( point( i, j ) ) == t_floor ) {
                if( one_in( 15 ) ) {
                    m.add_spawn( spider_type, rng( 1, 2 ), tripoint( i, j, loc.z ) );
                    for( int x = i - 1; x <= i + 1; x++ ) {
                        for( int y = j - 1; y <= j + 1; y++ ) {
                            if( m.ter( point( x, y ) ) == t_floor ) {
                                madd_field( &m, point( x, y ), fd_web, rng( 2, 3 ) );
                                if( one_in( 4 ) ) {
                                    m.furn_set( point( i, j ), egg_type );
                                    m.remove_field( {i, j, m.get_abs_sub().z()}, fd_web );
                                }
                            }
                        }
                    }
                } else if( m.passable( point( i, j ) ) && one_in( 5 ) ) {
                    madd_field( &m, point( i, j ), fd_web, 1 );
                }
            }
        }
    }
    m.place_items( Item_spawn_data_rare, 60, point_zero, point( SEEX * 2 - 1, SEEY * 2 - 1 ),
                   false, calendar::start_of_cataclysm );

    return true;
}

static bool mx_helicopter( map &m, const tripoint &abs_sub )
{
    point c( rng( 6, SEEX * 2 - 7 ), rng( 6, SEEY * 2 - 7 ) );

    for( int x = 0; x < SEEX * 2; x++ ) {
        for( int y = 0; y < SEEY * 2; y++ ) {
            if( m.veh_at( tripoint( x,  y, abs_sub.z ) ) &&
                m.ter( tripoint( x, y, abs_sub.z ) )->has_flag( ter_furn_flag::TFLAG_DIGGABLE ) ) {
                m.ter_set( tripoint( x, y, abs_sub.z ), t_dirtmound );
            } else {
                if( x >= c.x - dice( 1, 5 ) && x <= c.x + dice( 1, 5 ) && y >= c.y - dice( 1, 5 ) &&
                    y <= c.y + dice( 1, 5 ) ) {
                    if( one_in( 7 ) &&
                        m.ter( tripoint( x, y, abs_sub.z ) )->has_flag( ter_furn_flag::TFLAG_DIGGABLE ) ) {
                        m.ter_set( tripoint( x, y, abs_sub.z ), t_dirtmound );
                    }
                }
                if( x >= c.x - dice( 1, 6 ) && x <= c.x + dice( 1, 6 ) && y >= c.y - dice( 1, 6 ) &&
                    y <= c.y + dice( 1, 6 ) ) {
                    if( !one_in( 5 ) ) {
                        m.make_rubble( tripoint( x,  y, abs_sub.z ), f_wreckage, true );
                        if( m.ter( tripoint( x, y, abs_sub.z ) )->has_flag( ter_furn_flag::TFLAG_DIGGABLE ) ) {
                            m.ter_set( tripoint( x, y, abs_sub.z ), t_dirtmound );
                        }
                    } else if( m.is_bashable( point( x, y ) ) ) {
                        m.destroy( tripoint( x,  y, abs_sub.z ), true );
                        if( m.ter( tripoint( x, y, abs_sub.z ) )->has_flag( ter_furn_flag::TFLAG_DIGGABLE ) ) {
                            m.ter_set( tripoint( x, y, abs_sub.z ), t_dirtmound );
                        }
                    }

                } else if( one_in( 4 + ( std::abs( x - c.x ) + std::abs( y -
                                         c.y ) ) ) ) { // 1 in 10 chance of being wreckage anyway
                    m.make_rubble( tripoint( x,  y, abs_sub.z ), f_wreckage, true );
                    if( !one_in( 3 ) ) {
                        if( m.ter( tripoint( x, y, abs_sub.z ) )->has_flag( ter_furn_flag::TFLAG_DIGGABLE ) ) {
                            m.ter_set( tripoint( x, y, abs_sub.z ), t_dirtmound );
                        }
                    }
                }
            }
        }
    }

    units::angle dir1 = random_direction();

    auto crashed_hull = VehicleGroup_crashed_helicopters->pick();

    // Create the vehicle so we can rotate it and calculate its bounding box, but don't place it on the map.
    auto veh = std::make_unique<vehicle>( crashed_hull, rng( 1, 33 ), 1 );

    veh->turn( dir1 );

    // Get the bounding box, centered on mount(0,0)
    bounding_box bbox = veh->get_bounding_box();
    // Move the wreckage forward/backward half it's length so that it spawns more over the center of the debris area
    point length( std::abs( bbox.p2.x - bbox.p1.x ), std::abs( bbox.p2.y - bbox.p1.y ) );

    // cont.
    point offset( veh->dir_vec().x * length.x / 2, veh->dir_vec().y * length.y / 2 );

    point min( std::abs( bbox.p1.x ) + 0, std::abs( bbox.p1.y ) + 0 );

    int x_max = SEEX * 2 - bbox.p2.x - 1;
    int y_max = SEEY * 2 - bbox.p2.y - 1;

    // Clamp x1 & y1 such that no parts of the vehicle extend over the border of the submap.
    point p1( clamp( c.x + offset.x, min.x, x_max ), clamp( c.y + offset.y, min.y, y_max ) );

    vehicle *wreckage = m.add_vehicle(
                            crashed_hull, tripoint( p1, abs_sub.z ), dir1, rng( 1, 33 ), 1 );

    const auto controls_at = []( vehicle * wreckage, const tripoint & pos ) {
        return !wreckage->get_parts_at( pos, "CONTROLS", part_status_flag::any ).empty() ||
               !wreckage->get_parts_at( pos, "CTRL_ELECTRONIC", part_status_flag::any ).empty();
    };

    if( wreckage != nullptr ) {
        const int clowncar_factor = dice( 1, 8 );

        switch( clowncar_factor ) {
            case 1:
            case 2:
            case 3:
                // Full clown car
                for( const vpart_reference &vp : wreckage->get_any_parts( VPFLAG_SEATBELT ) ) {
                    const tripoint pos = vp.pos();
                    // Spawn pilots in seats with controls.CTRL_ELECTRONIC
                    if( controls_at( wreckage, pos ) ) {
                        m.add_spawn( mon_zombie_military_pilot, 1, pos );
                    } else {
                        if( one_in( 5 ) ) {
                            m.add_spawn( mon_zombie_bio_op, 1, pos );
                        } else if( one_in( 5 ) ) {
                            m.add_spawn( mon_zombie_scientist, 1, pos );
                        } else {
                            m.add_spawn( mon_zombie_soldier, 1, pos );
                        }
                    }

                    // Delete the items that would have spawned here from a "corpse"
                    for( int sp : wreckage->parts_at_relative( vp.mount(), true ) ) {
                        vehicle_stack here = wreckage->get_items( sp );

                        for( auto iter = here.begin(); iter != here.end(); ) {
                            iter = here.erase( iter );
                        }
                    }
                }
                break;
            case 4:
            case 5:
                // 2/3rds clown car
                for( const vpart_reference &vp : wreckage->get_any_parts( VPFLAG_SEATBELT ) ) {
                    const tripoint pos = vp.pos();
                    // Spawn pilots in seats with controls.
                    if( controls_at( wreckage, pos ) ) {
                        m.add_spawn( mon_zombie_military_pilot, 1, pos );
                    } else {
                        if( !one_in( 3 ) ) {
                            m.add_spawn( mon_zombie_soldier, 1, pos );
                        }
                    }

                    // Delete the items that would have spawned here from a "corpse"
                    for( int sp : wreckage->parts_at_relative( vp.mount(), true ) ) {
                        vehicle_stack here = wreckage->get_items( sp );

                        for( auto iter = here.begin(); iter != here.end(); ) {
                            iter = here.erase( iter );
                        }
                    }
                }
                break;
            case 6:
                // Just pilots
                for( const vpart_reference &vp : wreckage->get_any_parts( VPFLAG_CONTROLS ) ) {
                    const tripoint pos = vp.pos();
                    m.add_spawn( mon_zombie_military_pilot, 1, pos );

                    // Delete the items that would have spawned here from a "corpse"
                    for( int sp : wreckage->parts_at_relative( vp.mount(), true ) ) {
                        vehicle_stack here = wreckage->get_items( sp );

                        for( auto iter = here.begin(); iter != here.end(); ) {
                            iter = here.erase( iter );
                        }
                    }
                }
                break;
            case 7:
            // Empty clown car
            case 8:
            default:
                break;
        }
        if( !one_in( 4 ) ) {
            wreckage->smash( m, 0.8f, 1.2f, 1.0f, point( dice( 1, 8 ) - 5, dice( 1, 8 ) - 5 ), 6 + dice( 1,
                             10 ) );
        } else {
            wreckage->smash( m, 0.1f, 0.9f, 1.0f, point( dice( 1, 8 ) - 5, dice( 1, 8 ) - 5 ), 6 + dice( 1,
                             10 ) );
        }
    }

    return true;
}

static bool mx_military( map &m, const tripoint & )
{
    int num_bodies = dice( 2, 6 );
    for( int i = 0; i < num_bodies; i++ ) {
        if( const auto p = random_point( m, [&m]( const tripoint & n ) {
        return m.passable( n );
        } ) ) {
            if( one_in( 25 ) ) {
                if( one_in( 2 ) ) {
                    m.add_spawn( mon_zombie_bio_op, 1, *p );

                    int splatter_range = rng( 1, 3 );
                    for( int j = 0; j <= splatter_range; j++ ) {
                        m.add_field( *p + point( -j * 1, j * 1 ), fd_blood, 1, 0_turns );
                    }
                } else {
                    m.add_spawn( mon_dispatch, 1, *p );
                }
            } else {
                m.add_spawn( mon_zombie_soldier, 1, *p );
                // 10% chance of zombie carrying weapon so 90% chance of it being on the ground
                if( !one_in( 10 ) ) {
                    item_group_id group;
                    // 80% assault rifles, 10% LMGs, 5% shotguns, 5% sniper rifles
                    if( one_in( 20 ) ) {
                        group = Item_spawn_data_military_standard_sniper_rifles;
                    } else if( one_in( 19 ) ) {
                        group = Item_spawn_data_military_standard_shotguns;
                    } else if( one_in( 9 ) ) {
                        group = Item_spawn_data_military_standard_lmgs;
                    } else {
                        group = Item_spawn_data_military_standard_assault_rifles;
                    }
                    m.place_items( group, 100, *p, *p, true, calendar::start_of_cataclysm );
                }

                int splatter_range = rng( 1, 3 );
                for( int j = 0; j <= splatter_range; j++ ) {
                    m.add_field( *p + point( -j * 1, j * 1 ), fd_blood, 1, 0_turns );
                }
            }
        }

    }
    int num_monsters = rng( 0, 3 );
    for( int i = 0; i < num_monsters; i++ ) {
        point m2( rng( 1, SEEX * 2 - 2 ), rng( 1, SEEY * 2 - 2 ) );
        m.place_spawns( GROUP_NETHER_CAPTURED, 1, m2, m2, 1, true );
    }
    m.place_spawns( GROUP_MAYBE_MIL, 2, point_zero, point( SEEX * 2 - 1, SEEY * 2 - 1 ),
                    0.1f ); //0.1 = 1-5
    m.place_items( Item_spawn_data_rare, 25, point_zero, point( SEEX * 2 - 1, SEEY * 2 - 1 ),
                   true, calendar::start_of_cataclysm );

    return true;
}

static bool mx_roadblock( map &m, const tripoint &abs_sub )
{
    // TODO: fix point types
    const tripoint_abs_omt abs_omt( sm_to_omt_copy( abs_sub ) );
    const oter_id &north = overmap_buffer.ter( abs_omt + point_north );
    const oter_id &south = overmap_buffer.ter( abs_omt + point_south );
    const oter_id &west = overmap_buffer.ter( abs_omt + point_west );
    const oter_id &east = overmap_buffer.ter( abs_omt + point_east );

    const bool road_at_north = north->get_type_id() == oter_type_road;
    const bool road_at_south = south->get_type_id() == oter_type_road;
    const bool road_at_west = west->get_type_id() == oter_type_road;
    const bool road_at_east = east->get_type_id() == oter_type_road;

    const auto spawn_turret = [&]( const point & p ) {
        m.add_spawn( mon_turret_riot, 1, { p, abs_sub.z } );
    };

    if( one_in( 6 ) ) { //Military doesn't joke around with their barricades!

        if( one_in( 2 ) ) {
            if( road_at_north ) {
                line( &m, t_fence_barbed, point( 4, 3 ), point( 10, 3 ) );
                line( &m, t_fence_barbed, point( 13, 3 ), point( 19, 3 ) );
            }
            if( road_at_east ) {
                line( &m, t_fence_barbed, point( SEEX * 2 - 3, 4 ), point( SEEX * 2 - 3, 10 ) );
                line( &m, t_fence_barbed, point( SEEX * 2 - 3, 13 ), point( SEEX * 2 - 3, 19 ) );
            }
            if( road_at_south ) {
                line( &m, t_fence_barbed, point( 4, SEEY * 2 - 3 ), point( 10, SEEY * 2 - 3 ) );
                line( &m, t_fence_barbed, point( 13, SEEY * 2 - 3 ), point( 19, SEEY * 2 - 3 ) );
            }
            if( road_at_west ) {
                line( &m, t_fence_barbed, point( 3, 4 ), point( 3, 10 ) );
                line( &m, t_fence_barbed, point( 3, 13 ), point( 3, 19 ) );
            }
        } else {
            if( road_at_north ) {
                line_furn( &m, f_sandbag_half, point( 4, 3 ), point( 10, 3 ) );
                line_furn( &m, f_sandbag_half, point( 13, 3 ), point( 19, 3 ) );
            }
            if( road_at_east ) {
                line_furn( &m, f_sandbag_half, point( SEEX * 2 - 3, 4 ), point( SEEX * 2 - 3, 10 ) );
                line_furn( &m, f_sandbag_half, point( SEEX * 2 - 3, 13 ), point( SEEX * 2 - 3, 19 ) );
            }
            if( road_at_south ) {
                line_furn( &m, f_sandbag_half, point( 4, SEEY * 2 - 3 ), point( 10, SEEY * 2 - 3 ) );
                line_furn( &m, f_sandbag_half, point( 13, SEEY * 2 - 3 ), point( 19, SEEY * 2 - 3 ) );
            }
            if( road_at_east ) {
                line_furn( &m, f_sandbag_half, point( 3, 4 ), point( 3, 10 ) );
                line_furn( &m, f_sandbag_half, point( 3, 13 ), point( 3, 19 ) );
            }
        }

        if( one_in( 2 ) ) {
            // The truck's wrecked...with fuel.  Explosive barrel?
            m.add_vehicle( vehicle_prototype_military_cargo_truck, point( 12, SEEY * 2 - 12 ),
                           0_degrees, 70, -1 );
            if( road_at_north ) {
                spawn_turret( point( 12, 6 ) );
            }
            if( road_at_east ) {
                spawn_turret( point( 18, 12 ) );
            }
            if( road_at_south ) {
                spawn_turret( point( 12, 18 ) );
            }
            if( road_at_west ) {
                spawn_turret( point( 6, 12 ) );
            }
        } else {  // Vehicle & turrets
            m.add_vehicle( VehicleGroup_military_vehicles,
                           tripoint( 12, SEEY * 2 - 10, abs_sub.z ), 0_degrees, 70, -1 );
            if( road_at_north ) {
                spawn_turret( point( 12, 6 ) );
            }
            if( road_at_east ) {
                spawn_turret( point( 18, 12 ) );
            }
            if( road_at_south ) {
                spawn_turret( point( 12, 18 ) );
            }
            if( road_at_west ) {
                spawn_turret( point( 6, 12 ) );
            }
        }

        line_furn( &m, f_sandbag_wall, point( 12, 7 ), point( 15, 7 ) );
        m.add_spawn( mon_turret_searchlight, 1, { 13, 8, abs_sub.z } );
        m.ter_set( point( 14, 8 ), t_plut_generator );
        line_furn( &m, f_sandbag_wall, point( 12, 9 ), point( 15, 9 ) );

        int num_bodies = dice( 2, 5 );
        for( int i = 0; i < num_bodies; i++ ) {
            if( const auto p = random_point( m, [&m]( const tripoint & n ) {
            return m.passable( n );
            } ) ) {
                m.add_spawn( mon_zombie_soldier, 1, *p );
                // 10% chance of zombie carrying weapon so 90% chance of it being on the ground
                if( !one_in( 10 ) ) {
                    item_group_id group;
                    // 75% assault rifles, 10% LMGs, 5% shotguns, 5% sniper rifles
                    if( one_in( 20 ) ) {
                        group = Item_spawn_data_military_standard_sniper_rifles;
                    } else if( one_in( 20 ) ) {
                        group = Item_spawn_data_military_standard_shotguns;
                    } else if( one_in( 10 ) ) {
                        group = Item_spawn_data_military_standard_lmgs;
                    } else {
                        group = Item_spawn_data_military_standard_assault_rifles;
                    }
                    m.place_items( group, 100, *p, *p, true, calendar::start_of_cataclysm );
                }

                int splatter_range = rng( 1, 3 );
                for( int j = 0; j <= splatter_range; j++ ) {
                    m.add_field( *p + point( -j * 1, j * 1 ), fd_blood, 1, 0_turns );
                }
            }
        }
    } else { // Police roadblock

        if( road_at_north ) {
            line_furn( &m, f_barricade_road, point( 4, 3 ), point( 10, 3 ) );
            line_furn( &m, f_barricade_road, point( 13, 3 ), point( 19, 3 ) );
            m.add_spawn( mon_turret_riot, 1, { 12, 1, abs_sub.z } );
        }
        if( road_at_east ) {
            line_furn( &m, f_barricade_road, point( SEEX * 2 - 3, 4 ), point( SEEX * 2 - 3, 10 ) );
            line_furn( &m, f_barricade_road, point( SEEX * 2 - 3, 13 ), point( SEEX * 2 - 3, 19 ) );
            m.add_spawn( mon_turret_riot, 1, { SEEX * 2 - 1, 12, abs_sub.z } );
        }
        if( road_at_south ) {
            line_furn( &m, f_barricade_road, point( 4, SEEY * 2 - 3 ), point( 10, SEEY * 2 - 3 ) );
            line_furn( &m, f_barricade_road, point( 13, SEEY * 2 - 3 ), point( 19, SEEY * 2 - 3 ) );
            m.add_spawn( mon_turret_riot, 1, { 12, SEEY * 2 - 1, abs_sub.z } );
        }
        if( road_at_west ) {
            line_furn( &m, f_barricade_road, point( 3, 4 ), point( 3, 10 ) );
            line_furn( &m, f_barricade_road, point( 3, 13 ), point( 3, 19 ) );
            m.add_spawn( mon_turret_riot, 1, { 1, 12, abs_sub.z } );
        }

        m.add_vehicle( vehicle_prototype_policecar, point( 8, 6 ), 20_degrees );
        m.add_vehicle( vehicle_prototype_policecar, point( 16, SEEY * 2 - 6 ), 145_degrees );

        line_furn( &m, f_sandbag_wall, point( 6, 10 ), point( 9, 10 ) );
        m.add_spawn( mon_turret_searchlight, 1, { 7, 11, abs_sub.z } );
        m.ter_set( point( 8, 11 ), t_plut_generator );
        line_furn( &m, f_sandbag_wall, point( 6, 12 ), point( 9, 12 ) );

        int num_bodies = dice( 1, 6 );
        for( int i = 0; i < num_bodies; i++ ) {
            if( const auto p = random_point( m, [&m]( const tripoint & n ) {
            return m.passable( n );
            } ) ) {
                m.place_items( Item_spawn_data_map_extra_police, 100, *p, *p, true,
                               calendar::start_of_cataclysm );

                int splatter_range = rng( 1, 3 );
                for( int j = 0; j <= splatter_range; j++ ) {
                    m.add_field( *p + point( j * 1, -j * 1 ), fd_blood, 1, 0_turns );
                }
            }
        }
    }

    return true;
}

static bool mx_bandits_block( map &m, const tripoint &abs_sub )
{
    const tripoint_abs_omt abs_omt( sm_to_omt_copy( abs_sub ) );
    const oter_id &north = overmap_buffer.ter( abs_omt + point_north );
    const oter_id &south = overmap_buffer.ter( abs_omt + point_south );
    const oter_id &west = overmap_buffer.ter( abs_omt + point_west );
    const oter_id &east = overmap_buffer.ter( abs_omt + point_east );

    const bool forest_at_north = is_ot_match( "forest", north, ot_match_type::prefix );
    const bool forest_at_south = is_ot_match( "forest", south, ot_match_type::prefix );
    const bool forest_at_west = is_ot_match( "forest", west, ot_match_type::prefix );
    const bool forest_at_east = is_ot_match( "forest", east, ot_match_type::prefix );

    const bool road_at_north = north->get_type_id() == oter_type_road;
    const bool road_at_south = south->get_type_id() == oter_type_road;
    const bool road_at_west = west->get_type_id() == oter_type_road;
    const bool road_at_east = east->get_type_id() == oter_type_road;

    if( forest_at_north && forest_at_south &&
        road_at_west && road_at_east ) {
        if( one_in( 2 ) ) {
            line( &m, t_trunk, point( 1, 3 ), point( 1, 6 ) );
            line( &m, t_trunk, point( 1, 8 ), point( 1, 13 ) );
            line( &m, t_trunk, point( 2, 14 ), point( 2, 17 ) );
            line( &m, t_trunk, point( 1, 18 ), point( 2, 22 ) );
            m.ter_set( point( 1, 2 ), t_stump );
            m.ter_set( point( 1, 20 ), t_stump );
            m.ter_set( point_south_east, t_improvised_shelter );
            m.place_npc( point( 2, 19 ), npc_template_bandit );
            if( one_in( 2 ) ) {
                m.place_npc( point_south_east, npc_template_bandit );
            }
        } else {
            trap_str_id trap_type = one_in( 2 ) ? tr_nailboard : tr_caltrops;
            for( int x = SEEX - 1; x < SEEX + 1; x++ ) {
                for( int y = 0; y < SEEY * 2 - 1; y += 2 ) {
                    if( x_in_y( 8, 10 ) ) {
                        m.trap_set( { x, y, abs_sub.z }, trap_type );
                    }
                }
            }

            rough_circle( &m, t_underbrush, point( 8, 2 ), 2 );
            m.ter_set( point( 8, 2 ), t_dirt );
            m.place_npc( point( 8, 2 ), npc_template_bandit );

            rough_circle( &m, t_underbrush, point( 16, 22 ), 2 );
            m.ter_set( point( 16, 22 ), t_dirt );
            m.place_npc( point( 16, 22 ), npc_template_bandit );
        }

        return true;
    }

    if( forest_at_west && forest_at_east && road_at_north && road_at_south ) {
        if( one_in( 2 ) ) {
            // NOLINTNEXTLINE(cata-use-named-point-constants)
            line( &m, t_trunk, point( 1, 1 ), point( 3, 1 ) );
            line( &m, t_trunk, point( 5, 1 ), point( 10, 1 ) );
            line( &m, t_trunk, point( 11, 3 ), point( 16, 3 ) );
            line( &m, t_trunk, point( 17, 2 ), point( 21, 2 ) );
            m.ter_set( point( 22, 2 ), t_stump );
            m.ter_set( point_south, t_improvised_shelter );
            m.place_npc( point( 20, 3 ), npc_template_bandit );
            if( one_in( 2 ) ) {
                m.place_npc( point_south, npc_template_bandit );
            }
        } else {
            trap_str_id trap_type = one_in( 2 ) ? tr_nailboard : tr_caltrops;
            for( int x = 0; x < SEEX * 2 - 1; x += 2 ) {
                for( int y = SEEY - 1; y < SEEY + 1; y++ ) {
                    if( x_in_y( 8, 10 ) ) {
                        m.trap_set( { x, y, abs_sub.z }, trap_type );
                    }
                }
            }

            rough_circle( &m, t_underbrush, point( 1, 8 ), 2 );
            m.ter_set( point( 1, 8 ), t_dirt );
            m.place_npc( point( 1, 8 ), npc_template_bandit );

            rough_circle( &m, t_underbrush, point( 22, 15 ), 2 );
            m.ter_set( point( 22, 15 ), t_dirt );
            m.place_npc( point( 22, 15 ), npc_template_bandit );
        }

        return true;
    }

    return false;
}

static bool mx_supplydrop( map &m, const tripoint &/*abs_sub*/ )
{
    const bool intact = x_in_y( 40,
                                std::max( to_days<int>( calendar::turn - calendar::start_of_cataclysm ), 0 ) + 50 );

    int num_crates = rng( 1, 5 );
    for( int i = 0; i < num_crates; i++ ) {
        const auto p = random_point( m, [&m]( const tripoint & n ) {
            return m.passable( n );
        } );
        if( !p ) {
            break;
        }

        if( intact ) {
            m.furn_set( p->xy(), f_crate_c );
            item_group_id item_group;
            switch( rng( 1, 10 ) ) {
                case 1:
                    item_group = Item_spawn_data_mil_bulk;
                    break;
                case 2:
                case 3:
                case 4:
                    item_group = Item_spawn_data_mil_food;
                    break;
                case 5:
                case 6:
                case 7:
                    item_group = Item_spawn_data_grenades;
                    break;
                case 8:
                case 9:
                    item_group = Item_spawn_data_mil_armor;
                    break;
                case 10:
                    item_group = Item_spawn_data_guns_rifle_milspec;
                    break;
            }
            int items_created = 0;
            for( int i = 0; i < 10 && items_created < 2; i++ ) {
                items_created +=
                    m.place_items( item_group, 80, *p, *p, true, calendar::start_of_cataclysm,
                                   100 ).size();
            }
            if( m.i_at( *p ).empty() ) {
                m.destroy( *p, true );
            }
        } else {
            m.furn_set( p->xy(), f_crate_o );
        }
    }

    return true;
}

static void place_trap_if_clear( map &m, const point &target, trap_id trap_type )
{
    tripoint tri_target( target, m.get_abs_sub().z() );
    if( m.ter( tri_target ).obj().trap == tr_null ) {
        mtrap_set( &m, target, trap_type );
    }
}

static bool mx_minefield( map &, const tripoint &abs_sub )
{
    const tripoint_abs_omt abs_omt( sm_to_omt_copy( abs_sub ) );
    const oter_id &center = overmap_buffer.ter( abs_omt );
    const oter_id &north = overmap_buffer.ter( abs_omt + point_north );
    const oter_id &south = overmap_buffer.ter( abs_omt + point_south );
    const oter_id &west = overmap_buffer.ter( abs_omt + point_west );
    const oter_id &east = overmap_buffer.ter( abs_omt + point_east );

    const bool bridgehead_at_center = center->get_type_id() ==
                                      oter_type_bridgehead_ground;
    const bool bridge_at_north = north->get_type_id() == oter_type_bridge;
    const bool bridge_at_south = south->get_type_id() == oter_type_bridge;
    const bool bridge_at_west = west->get_type_id() == oter_type_bridge;
    const bool bridge_at_east = east->get_type_id() == oter_type_bridge;

    const bool road_at_north = north->get_type_id() == oter_type_road;
    const bool road_at_south = south->get_type_id() == oter_type_road;
    const bool road_at_west = west->get_type_id() == oter_type_road;
    const bool road_at_east = east->get_type_id() == oter_type_road;

    const int num_mines = rng( 6, 20 );
    const std::string text = _( "DANGER!  MINEFIELD!" );

    bool did_something = false;

    if( !bridgehead_at_center ) {
        return false;
    }

    tinymap m;
    if( bridge_at_north && road_at_south ) {
        m.load( project_to<coords::sm>( abs_omt + point_south ), false );

        //Sandbag block at the left edge
        line_furn( &m, f_sandbag_half, point( 3, 4 ), point( 3, 7 ) );
        line_furn( &m, f_sandbag_half, point( 3, 7 ), point( 9, 7 ) );
        line_furn( &m, f_sandbag_half, point( 9, 4 ), point( 9, 7 ) );

        //7.62x51mm casings left from m60 of the humvee
        for( const auto &loc : m.points_in_radius( { 6, 4, abs_sub.z }, 3, 0 ) ) {
            if( one_in( 4 ) ) {
                m.spawn_item( loc, itype_762_51_casing );
            }
        }

        //50% chance to spawn a humvee in the left block
        if( one_in( 2 ) ) {
            m.add_vehicle( vehicle_prototype_humvee, point( 5, 3 ), 270_degrees, 70, -1 );
        }

        //Sandbag block at the right edge
        line_furn( &m, f_sandbag_half, point( 15, 3 ), point( 15, 6 ) );
        line_furn( &m, f_sandbag_half, point( 15, 6 ), point( 20, 6 ) );
        line_furn( &m, f_sandbag_half, point( 20, 3 ), point( 20, 6 ) );

        //5.56x45mm casings left from a soldier
        for( const auto &loc : m.points_in_radius( { 17, 4, abs_sub.z }, 2, 0 ) ) {
            if( one_in( 4 ) ) {
                m.spawn_item( loc, itype_223_casing );
            }
        }

        //50% chance to spawn a dead soldier with a trail of blood
        if( one_in( 2 ) ) {
            m.add_splatter_trail( fd_blood, { 17, 6, abs_sub.z }, { 19, 3, abs_sub.z } );
            item body = item::make_corpse();
            m.put_items_from_loc( Item_spawn_data_mon_zombie_soldier_death_drops,
            { 17, 5, abs_sub.z } );
            m.add_item_or_charges( { 17, 5, abs_sub.z }, body );
        }

        //33% chance to spawn empty magazines used by soldiers
        std::vector<point> empty_magazines_locations = line_to( point( 15, 5 ), point( 20, 5 ) );
        for( auto &i : empty_magazines_locations ) {
            if( one_in( 3 ) ) {
                m.spawn_item( { i, abs_sub.z }, itype_stanag30 );
            }
        }

        //Horizontal line of barbed wire fence
        line( &m, t_fence_barbed, point( 3, 9 ), point( SEEX * 2 - 4, 9 ) );

        std::vector<point> barbed_wire = line_to( point( 3, 9 ), point( SEEX * 2 - 4, 9 ) );
        for( auto &i : barbed_wire ) {
            //10% chance to spawn corpses of bloody people/zombies on every tile of barbed wire fence
            if( one_in( 10 ) ) {
                m.add_corpse( { i, abs_sub.z } );
                m.add_field( { i, abs_sub.z }, fd_blood, rng( 1, 3 ) );
            }
        }

        //Spawn 6-20 mines in the lower submap.
        //Spawn ordinary mine on asphalt, otherwise spawn buried mine
        for( int i = 0; i < num_mines; i++ ) {
            const point p( rng( 3, SEEX * 2 - 4 ), rng( SEEY, SEEY * 2 - 2 ) );
            if( m.has_flag( ter_furn_flag::TFLAG_DIGGABLE, p ) ) {
                place_trap_if_clear( m, p, tr_landmine_buried );
            } else {
                place_trap_if_clear( m, p, tr_landmine );
            }
        }

        //Spawn 6-20 puddles of blood on tiles without mines
        for( int i = 0; i < num_mines; i++ ) {
            const point p2( rng( 3, SEEX * 2 - 4 ), rng( SEEY, SEEY * 2 - 2 ) );
            if( m.tr_at( { p2, abs_sub.z } ).is_null() ) {
                m.add_field( { p2, abs_sub.z }, fd_blood, rng( 1, 3 ) );
                //10% chance to spawn a corpse of dead people/zombie on a tile with blood
                if( one_in( 10 ) ) {
                    m.add_corpse( { p2, abs_sub.z } );
                    for( const auto &loc : m.points_in_radius( { p2, abs_sub.z }, 1 ) ) {
                        //50% chance to spawn gibs in every tile around corpse in 1-tile radius
                        if( one_in( 2 ) ) {
                            m.add_field( { loc.xy(), abs_sub.z }, fd_gibs_flesh, rng( 1, 3 ) );
                        }
                    }
                }
            }
        }

        //Set two warning signs on the last horizontal line of the submap
        const int x = rng( 3, SEEX );
        const int x1 = rng( SEEX + 1, SEEX * 2 - 4 );
        m.furn_set( point( x, SEEY * 2 - 1 ), furn_f_sign_warning );
        m.set_signage( tripoint( x, SEEY * 2 - 1, abs_sub.z ), text );
        m.furn_set( point( x1, SEEY * 2 - 1 ), furn_f_sign_warning );
        m.set_signage( tripoint( x1, SEEY * 2 - 1, abs_sub.z ), text );

        did_something = true;
    }

    if( bridge_at_south && road_at_north ) {
        m.load( project_to<coords::sm>( abs_omt + point_north ), false );
        //Two horizontal lines of sandbags
        line_furn( &m, f_sandbag_half, point( 5, 15 ), point( 10, 15 ) );
        line_furn( &m, f_sandbag_half, point( 13, 15 ), point( 18, 15 ) );

        //Section of barbed wire fence
        line( &m, t_fence_barbed, point( 3, 13 ), point( SEEX * 2 - 4, 13 ) );

        std::vector<point> barbed_wire = line_to( point( 3, 13 ), point( SEEX * 2 - 4, 13 ) );
        for( auto &i : barbed_wire ) {
            //10% chance to spawn corpses of bloody people/zombies on every tile of barbed wire fence
            if( one_in( 10 ) ) {
                m.add_corpse( { i, abs_sub.z } );
                m.add_field( { i, abs_sub.z }, fd_blood, rng( 1, 3 ) );
            }
        }

        //50% chance to spawn a blood trail of wounded soldier trying to escape,
        //but eventually died out of blood loss and wounds and got devoured by zombies
        if( one_in( 2 ) ) {
            m.add_splatter_trail( fd_blood, { 9, 15, abs_sub.z }, { 11, 18, abs_sub.z } );
            m.add_splatter_trail( fd_blood, { 11, 18, abs_sub.z }, { 11, 21, abs_sub.z } );
            for( const auto &loc : m.points_in_radius( { 11, 21, abs_sub.z }, 1 ) ) {
                //50% chance to spawn gibs in every tile around corpse in 1-tile radius
                if( one_in( 2 ) ) {
                    m.add_field( { loc.xy(), abs_sub.z }, fd_gibs_flesh, rng( 1, 3 ) );
                }
            }
            item body = item::make_corpse();
            m.put_items_from_loc( Item_spawn_data_mon_zombie_soldier_death_drops,
            { 11, 21, abs_sub.z } );
            m.add_item_or_charges( { 11, 21, abs_sub.z }, body );
        }

        //5.56x45mm casings left from a soldier
        for( const auto &loc : m.points_in_radius( { 9, 15, abs_sub.z }, 2, 0 ) ) {
            if( one_in( 4 ) ) {
                m.spawn_item( loc, itype_223_casing );
            }
        }

        //5.56x45mm casings left from another soldier
        for( const auto &loc : m.points_in_radius( { 15, 15, abs_sub.z }, 2, 0 ) ) {
            if( one_in( 4 ) ) {
                m.spawn_item( loc, itype_223_casing );
            }
        }

        //33% chance to spawn empty magazines used by soldiers
        std::vector<point> empty_magazines_locations = line_to( point( 5, 16 ), point( 18, 16 ) );
        for( auto &i : empty_magazines_locations ) {
            if( one_in( 3 ) ) {
                m.spawn_item( { i, abs_sub.z }, itype_stanag30 );
            }
        }

        //50% chance to spawn two humvees blocking the road
        if( one_in( 2 ) ) {
            m.add_vehicle( vehicle_prototype_humvee, point( 7, 19 ), 0_degrees, 70, -1 );
            m.add_vehicle( vehicle_prototype_humvee, point( 15, 20 ), 180_degrees, 70, -1 );
        }

        //Spawn 6-20 mines in the upper submap.
        //Spawn ordinary mine on asphalt, otherwise spawn buried mine
        for( int i = 0; i < num_mines; i++ ) {
            const point p3( rng( 3, SEEX * 2 - 4 ), rng( 1, SEEY ) );
            if( m.has_flag( ter_furn_flag::TFLAG_DIGGABLE, p3 ) ) {
                place_trap_if_clear( m, p3, tr_landmine_buried );
            } else {
                place_trap_if_clear( m, p3, tr_landmine );
            }
        }

        //Spawn 6-20 puddles of blood on tiles without mines
        for( int i = 0; i < num_mines; i++ ) {
            const point p4( rng( 3, SEEX * 2 - 4 ), rng( 1, SEEY ) );
            if( m.tr_at( { p4, abs_sub.z } ).is_null() ) {
                m.add_field( { p4, abs_sub.z }, fd_blood, rng( 1, 3 ) );
                //10% chance to spawn a corpse of dead people/zombie on a tile with blood
                if( one_in( 10 ) ) {
                    m.add_corpse( { p4, abs_sub.z } );
                    for( const auto &loc : m.points_in_radius( { p4, abs_sub.z }, 1 ) ) {
                        //50% chance to spawn gibs in every tile around corpse in 1-tile radius
                        if( one_in( 2 ) ) {
                            m.add_field( { loc.xy(), abs_sub.z }, fd_gibs_flesh, rng( 1, 3 ) );
                        }
                    }
                }
            }
        }

        //Set two warning signs on the first horizontal line of the submap
        const int x = rng( 3, SEEX );
        const int x1 = rng( SEEX + 1, SEEX * 2 - 4 );
        m.furn_set( point( x, 0 ), furn_f_sign_warning );
        m.set_signage( tripoint( x, 0, abs_sub.z ), text );
        m.furn_set( point( x1, 0 ), furn_f_sign_warning );
        m.set_signage( tripoint( x1, 0, abs_sub.z ), text );

        did_something = true;
    }

    if( bridge_at_west && road_at_east ) {
        m.load( project_to<coords::sm>( abs_omt + point_east ), false );
        //Draw walls of first tent
        square_furn( &m, f_canvas_wall, point( 0, 3 ), point( 4, 13 ) );

        //Add first tent doors
        m.furn_set( { 4, 5, abs_sub.z }, f_canvas_door );
        m.furn_set( { 4, 11, abs_sub.z }, f_canvas_door );

        //Fill empty space with groundsheets
        square_furn( &m, f_fema_groundsheet, point( 1, 4 ), point( 3, 12 ) );

        //Place makeshift beds in the first tent and place loot
        m.furn_set( { 1, 4, abs_sub.z }, f_makeshift_bed );
        m.put_items_from_loc( Item_spawn_data_army_bed, { 1, 4, abs_sub.z } );
        m.furn_set( { 1, 6, abs_sub.z }, f_makeshift_bed );
        m.furn_set( { 1, 8, abs_sub.z }, f_makeshift_bed );
        m.furn_set( { 1, 10, abs_sub.z }, f_makeshift_bed );
        m.put_items_from_loc( Item_spawn_data_army_bed, { 1, 10, abs_sub.z } );
        m.furn_set( { 1, 12, abs_sub.z }, f_makeshift_bed );
        m.put_items_from_loc( Item_spawn_data_army_bed, { 1, 12, abs_sub.z } );

        //33% chance for a crazy maniac ramming the tent with some unfortunate inside
        if( one_in( 3 ) ) {
            //Blood and gore
            std::vector<point> blood_track = line_to( point( 1, 6 ), point( 8, 6 ) );
            for( auto &i : blood_track ) {
                m.add_field( { i, abs_sub.z }, fd_blood, 1 );
            }
            m.add_field( { 1, 6, abs_sub.z }, fd_gibs_flesh, 1 );

            //Add the culprit
            m.add_vehicle( vehicle_prototype_car_fbi, point( 7, 7 ), 0_degrees, 70, 1 );

            //Remove tent parts after drive-through
            square_furn( &m, f_null, point( 0, 6 ), point( 8, 9 ) );

            //Add sandbag barricade and then destroy few sections where car smashed it
            line_furn( &m, f_sandbag_half, point( 10, 3 ), point( 10, 13 ) );
            line_furn( &m, f_null, point( 10, 7 ), point( 10, 8 ) );

            //Spill sand from damaged sandbags
            std::vector<point> sandbag_positions = squares_in_direction( point( 10, 7 ), point( 11, 8 ) );
            for( auto &i : sandbag_positions ) {
                m.spawn_item( { i, abs_sub.z }, itype_bag_canvas, rng( 5, 13 ) );
                m.spawn_item( { i, abs_sub.z }, itype_material_sand, rng( 3, 8 ) );
            }
        } else {
            m.put_items_from_loc( Item_spawn_data_army_bed, { 1, 6, abs_sub.z } );
            m.put_items_from_loc( Item_spawn_data_army_bed, { 1, 8, abs_sub.z } );

            //5.56x45mm casings left from a soldier
            for( const auto &loc : m.points_in_radius( { 9, 8, abs_sub.z }, 2, 0 ) ) {
                if( one_in( 4 ) ) {
                    m.spawn_item( loc, itype_223_casing );
                }
            }

            //33% chance to spawn empty magazines used by soldiers
            std::vector<point> empty_magazines_locations = line_to( point( 9, 3 ), point( 9, 13 ) );
            for( auto &i : empty_magazines_locations ) {
                if( one_in( 3 ) ) {
                    m.spawn_item( { i, abs_sub.z }, itype_stanag30 );
                }
            }
            //Intact sandbag barricade
            line_furn( &m, f_sandbag_half, point( 10, 3 ), point( 10, 13 ) );
        }

        //Add sandbags and barbed wire fence barricades
        line( &m, t_fence_barbed, point( 12, 3 ), point( 12, 13 ) );
        line_furn( &m, f_sandbag_half, point( 10, 16 ), point( 10, 20 ) );
        line( &m, t_fence_barbed, point( 12, 16 ), point( 12, 20 ) );

        //Place second tent
        square_furn( &m, f_canvas_wall, point( 0, 16 ), point( 4, 20 ) );
        square_furn( &m, f_fema_groundsheet, point( 1, 17 ), point( 3, 19 ) );
        m.furn_set( { 4, 18, abs_sub.z }, f_canvas_door );

        //Place desk and chair in the second tent
        line_furn( &m, f_desk, point( 1, 17 ), point( 2, 17 ) );
        m.furn_set( { 1, 18, abs_sub.z }, f_chair );

        //5.56x45mm casings left from another soldier
        for( const auto &loc : m.points_in_radius( { 9, 18, abs_sub.z }, 2, 0 ) ) {
            if( one_in( 4 ) ) {
                m.spawn_item( loc, itype_223_casing );
            }
        }

        //33% chance to spawn empty magazines used by soldiers
        std::vector<point> empty_magazines_locations = line_to( point( 9, 16 ), point( 9, 20 ) );
        for( auto &i : empty_magazines_locations ) {
            if( one_in( 3 ) ) {
                m.spawn_item( { i, abs_sub.z }, itype_stanag30 );
            }
        }

        std::vector<point> barbed_wire = line_to( point( 12, 3 ), point( 12, 20 ) );
        for( auto &i : barbed_wire ) {
            //10% chance to spawn corpses of bloody people/zombies on every tile of barbed wire fence
            if( one_in( 10 ) ) {
                m.add_corpse( { i, abs_sub.z } );
                m.add_field( { i, abs_sub.z }, fd_blood, rng( 1, 3 ) );
            }
        }

        //Spawn 6-20 mines in the rightmost submap.
        //Spawn ordinary mine on asphalt, otherwise spawn buried mine
        for( int i = 0; i < num_mines; i++ ) {
            const point p5( rng( SEEX + 1, SEEX * 2 - 2 ), rng( 3, SEEY * 2 - 4 ) );
            if( m.has_flag( ter_furn_flag::TFLAG_DIGGABLE, p5 ) ) {
                place_trap_if_clear( m, p5, tr_landmine_buried );
            } else {
                place_trap_if_clear( m, p5, tr_landmine );
            }
        }

        //Spawn 6-20 puddles of blood on tiles without mines
        for( int i = 0; i < num_mines; i++ ) {
            const point p6( rng( SEEX + 1, SEEX * 2 - 2 ), rng( 3, SEEY * 2 - 4 ) );
            if( m.tr_at( { p6, abs_sub.z } ).is_null() ) {
                m.add_field( { p6, abs_sub.z }, fd_blood, rng( 1, 3 ) );
                //10% chance to spawn a corpse of dead people/zombie on a tile with blood
                if( one_in( 10 ) ) {
                    m.add_corpse( { p6, abs_sub.z } );
                    for( const auto &loc : m.points_in_radius( { p6, abs_sub.z }, 1 ) ) {
                        //50% chance to spawn gibs in every tile around corpse in 1-tile radius
                        if( one_in( 2 ) ) {
                            m.add_field( { loc.xy(), abs_sub.z }, fd_gibs_flesh, rng( 1, 3 ) );
                        }
                    }
                }
            }
        }

        //Set two warning signs on the last vertical line of the submap
        const int y = rng( 3, SEEY );
        const int y1 = rng( SEEY + 1, SEEY * 2 - 4 );
        m.furn_set( point( SEEX * 2 - 1, y ), furn_f_sign_warning );
        m.set_signage( tripoint( SEEX * 2 - 1, y, abs_sub.z ), text );
        m.furn_set( point( SEEX * 2 - 1, y1 ), furn_f_sign_warning );
        m.set_signage( tripoint( SEEX * 2 - 1, y1, abs_sub.z ), text );

        did_something = true;
    }

    if( bridge_at_east && road_at_west ) {
        m.load( project_to<coords::sm>( abs_omt + point_west ), false );
        //Spawn military cargo truck blocking the entry
        m.add_vehicle( vehicle_prototype_military_cargo_truck, point( 15, 11 ), 270_degrees, 70, 1 );

        //Spawn sandbag barricades around the truck
        line_furn( &m, f_sandbag_half, point( 14, 3 ), point( 14, 8 ) );
        line_furn( &m, f_sandbag_half, point( 14, 17 ), point( 14, 20 ) );

        //50% chance to spawn a soldier killed by gunfire, and a trail of blood
        if( one_in( 2 ) ) {
            m.add_splatter_trail( fd_blood, { 14, 5, abs_sub.z }, { 17, 5, abs_sub.z } );
            item body = item::make_corpse();
            m.put_items_from_loc( Item_spawn_data_mon_zombie_soldier_death_drops,
            { 15, 5, abs_sub.z } );
            m.add_item_or_charges( { 15, 5, abs_sub.z }, body );
        }

        //5.56x45mm casings left from soldiers
        for( const auto &loc : m.points_in_radius( { 15, 5, abs_sub.z }, 2, 0 ) ) {
            if( one_in( 4 ) ) {
                m.spawn_item( loc, itype_223_casing );
            }
        }

        //33% chance to spawn empty magazines used by soldiers
        std::vector<point> empty_magazines_locations = line_to( point( 15, 2 ), point( 15, 8 ) );
        for( auto &i : empty_magazines_locations ) {
            if( one_in( 3 ) ) {
                m.spawn_item( { i, abs_sub.z }, itype_stanag30 );
            }
        }

        //Add some crates near the truck...
        m.furn_set( { 16, 18, abs_sub.z }, f_crate_c );
        m.furn_set( { 16, 19, abs_sub.z }, f_crate_c );
        m.furn_set( { 17, 18, abs_sub.z }, f_crate_o );

        //...and fill them with mines
        m.spawn_item( { 16, 18, abs_sub.z }, itype_landmine, rng( 0, 5 ) );
        m.spawn_item( { 16, 19, abs_sub.z }, itype_landmine, rng( 0, 5 ) );

        // Set some resting place with fire ring, camp chairs, folding table, and benches.
        m.furn_set( { 20, 12, abs_sub.z }, f_crate_o );
        m.furn_set( { 21, 12, abs_sub.z }, f_firering );
        m.furn_set( { 22, 12, abs_sub.z }, f_tourist_table );
        line_furn( &m, f_bench, point( 23, 11 ), point( 23, 13 ) );
        line_furn( &m, f_camp_chair, point( 20, 14 ), point( 21, 14 ) );

        m.spawn_item( { 21, 12, abs_sub.z }, itype_splinter, rng( 5, 10 ) );

        //33% chance for an argument between drunk soldiers gone terribly wrong
        if( one_in( 3 ) ) {
            m.spawn_item( { 22, 12, abs_sub.z }, itype_bottle_glass );
            m.spawn_item( { 23, 11, abs_sub.z }, itype_hatchet );

            //Spawn chopped soldier corpse
            item body = item::make_corpse();
            m.put_items_from_loc( Item_spawn_data_mon_zombie_soldier_death_drops,
            { 23, 12, abs_sub.z } );
            m.add_item_or_charges( { 23, 12, abs_sub.z }, body );
            m.add_field( { 23, 12, abs_sub.z }, fd_gibs_flesh, rng( 1, 3 ) );

            //Spawn broken bench and splintered wood
            m.furn_set( { 23, 13, abs_sub.z }, f_null );
            m.spawn_item( { 23, 13, abs_sub.z }, itype_splinter, rng( 5, 10 ) );

            //Spawn blood
            for( const auto &loc : m.points_in_radius( { 23, 12, abs_sub.z }, 1, 0 ) ) {
                if( one_in( 2 ) ) {
                    m.add_field( { loc.xy(), abs_sub.z }, fd_blood, rng( 1, 3 ) );
                }
            }
            //Spawn trash in a crate and its surroundings
            m.place_items( Item_spawn_data_trash_cart, 80, { 19, 11, abs_sub.z },
            { 21, 13, abs_sub.z }, false, calendar::start_of_cataclysm );
        } else {
            m.spawn_item( { 20, 11, abs_sub.z }, itype_hatchet );
            m.spawn_item( { 22, 12, abs_sub.z }, itype_vodka );
            m.spawn_item( { 20, 14, abs_sub.z }, itype_acoustic_guitar );

            //Spawn trash in a crate
            m.place_items( Item_spawn_data_trash_cart, 80, { 20, 12, abs_sub.z },
            { 20, 12, abs_sub.z }, false, calendar::start_of_cataclysm );
        }

        //Place a tent
        square_furn( &m, f_canvas_wall, point( 20, 4 ), point( 23, 7 ) );
        square_furn( &m, f_fema_groundsheet, point( 21, 5 ), point( 22, 6 ) );
        m.furn_set( { 21, 7, abs_sub.z }, f_canvas_door );

        //Place beds in a tent
        m.furn_set( { 21, 5, abs_sub.z }, f_makeshift_bed );
        m.put_items_from_loc( Item_spawn_data_army_bed, { 21, 5, abs_sub.z },
                              calendar::turn_zero );
        m.furn_set( { 22, 6, abs_sub.z }, f_makeshift_bed );
        m.put_items_from_loc( Item_spawn_data_army_bed, { 22, 6, abs_sub.z },
                              calendar::turn_zero );

        //Spawn 6-20 mines in the leftmost submap.
        //Spawn ordinary mine on asphalt, otherwise spawn buried mine
        for( int i = 0; i < num_mines; i++ ) {
            const point p7( rng( 1, SEEX ), rng( 3, SEEY * 2 - 4 ) );
            if( m.has_flag( ter_furn_flag::TFLAG_DIGGABLE, p7 ) ) {
                place_trap_if_clear( m, p7, tr_landmine_buried );
            } else {
                place_trap_if_clear( m, p7, tr_landmine );
            }
        }

        //Spawn 6-20 puddles of blood on tiles without mines
        for( int i = 0; i < num_mines; i++ ) {
            const point p8( rng( 1, SEEX ), rng( 3, SEEY * 2 - 4 ) );
            if( m.tr_at( { p8, abs_sub.z } ).is_null() ) {
                m.add_field( { p8, abs_sub.z }, fd_blood, rng( 1, 3 ) );
                //10% chance to spawn a corpse of dead people/zombie on a tile with blood
                if( one_in( 10 ) ) {
                    m.add_corpse( { p8, abs_sub.z } );
                    for( const auto &loc : m.points_in_radius( { p8, abs_sub.z }, 1 ) ) {
                        //50% chance to spawn gibs in every tile around corpse in 1-tile radius
                        if( one_in( 2 ) ) {
                            m.add_field( { loc.xy(), abs_sub.z }, fd_gibs_flesh, rng( 1, 3 ) );
                        }
                    }
                }
            }
        }

        //Set two warning signs on the first vertical line of the submap
        const int y = rng( 3, SEEY );
        const int y1 = rng( SEEY + 1, SEEY * 2 - 4 );
        m.furn_set( point( 0, y ), furn_f_sign_warning );
        m.set_signage( tripoint( 0, y, abs_sub.z ), text );
        m.furn_set( point( 0, y1 ), furn_f_sign_warning );
        m.set_signage( tripoint( 0, y1, abs_sub.z ), text );

        did_something = true;
    }

    return did_something;
}

static bool mx_crater( map &m, const tripoint &abs_sub )
{
    int size = rng( 2, 6 );
    int size_squared = size * size;
    point p( rng( size, SEEX * 2 - 1 - size ), rng( size, SEEY * 2 - 1 - size ) );
    for( int i = p.x - size; i <= p.x + size; i++ ) {
        for( int j = p.y - size; j <= p.y + size; j++ ) {
            //If we're using circular distances, make circular craters
            //Pythagoras to the rescue, x^2 + y^2 = hypotenuse^2
            if( !trigdist || ( i - p.x ) * ( i - p.x ) + ( j - p.y ) * ( j - p.y ) <= size_squared ) {
                m.destroy( tripoint( i,  j, abs_sub.z ), true );
            }
        }
    }

    return true;
}

static void place_fumarole( map &m, const point &p1, const point &p2, std::set<point> &ignited )
{
    // Tracks points nearby for ignition after the lava is placed
    //std::set<point> ignited;

    std::vector<point> fumarole = line_to( p1, p2, 0 );
    for( auto &i : fumarole ) {
        m.ter_set( i, t_lava );

        // Add all adjacent tiles (even on diagonals) for possible ignition
        // Since they're being added to a set, duplicates won't occur
        ignited.insert( i + point_north_west );
        ignited.insert( i + point_north );
        ignited.insert( i + point_north_east );
        ignited.insert( i + point_west );
        ignited.insert( i + point_east );
        ignited.insert( i + point_south_west );
        ignited.insert( i + point_south );
        ignited.insert( i + point_south_east );

        if( one_in( 6 ) ) {
            m.spawn_item( i + point_north_west, itype_chunk_sulfur );
        }
    }

}

static bool mx_portal_in( map &m, const tripoint &abs_sub )
{
    static constexpr int omt_size = SEEX * 2;
    // minimum 9 tiles from the edge because ARTPROP_FRACTAL calls
    // create_anomaly at an offset of 4, and create_anomaly generates a
    // furniture circle around that of radius 5.
    static constexpr int min_coord = 9;
    static constexpr int max_coord = omt_size - 1 - min_coord;
    static_assert( min_coord < max_coord, "no space for randomness" );
    const tripoint portal_location{
        rng( min_coord, max_coord ), rng( min_coord, max_coord ), abs_sub.z };
    const point p( portal_location.xy() );

    switch( rng( 1, 6 ) ) {
        //Mycus spreading through the portal
        case 1: {
            m.add_field( portal_location, fd_fatigue, 3 );
            fungal_effects fe;
            for( const auto &loc : m.points_in_radius( portal_location, 5 ) ) {
                if( one_in( 3 ) ) {
                    fe.marlossify( loc );
                }
            }
            //50% chance to spawn pouf-maker
            m.place_spawns( GROUP_FUNGI_FUNGALOID, 2, p + point_north_west, p + point_south_east, 1, true );
            break;
        }
        //Netherworld monsters spawning around the portal
        case 2: {
            m.add_field( portal_location, fd_fatigue, 3 );
            for( const auto &loc : m.points_in_radius( portal_location, 5 ) ) {
                m.place_spawns( GROUP_NETHER_PORTAL, 15, loc.xy(), loc.xy(), 1, true );
            }
            break;
        }
        //Several cracks in the ground originating from the portal
        case 3: {
            m.add_field( portal_location, fd_fatigue, 3 );
            for( int i = 0; i < rng( 1, 10 ); i++ ) {
                tripoint end_location = { rng( 0, SEEX * 2 - 1 ), rng( 0, SEEY * 2 - 1 ), abs_sub.z };
                std::vector<tripoint> failure = line_to( portal_location, end_location );
                for( auto &i : failure ) {
                    m.ter_set( { i.xy(), abs_sub.z }, t_pit );
                }
            }
            break;
        }
        //Radiation from the portal killed the vegetation
        case 4: {
            m.add_field( portal_location, fd_fatigue, 3 );
            const int rad = 5;
            for( int i = p.x - rad; i <= p.x + rad; i++ ) {
                for( int j = p.y - rad; j <= p.y + rad; j++ ) {
                    if( trig_dist( p, point( i, j ) ) + rng( 0, 3 ) <= rad ) {
                        const tripoint loc( i, j, abs_sub.z );
                        dead_vegetation_parser( m, loc );
                        m.adjust_radiation( loc.xy(), rng( 20, 40 ) );
                    }
                }
            }
            break;
        }
        //Lava seams originating from the portal
        case 5: {
            if( abs_sub.z <= 0 ) {
                point p1( rng( 1,    SEEX     - 3 ), rng( 1,    SEEY     - 3 ) );
                point p2( rng( SEEX, SEEX * 2 - 3 ), rng( SEEY, SEEY * 2 - 3 ) );
                // Pick a random cardinal direction to also spawn lava in
                // This will make the lava a single connected line, not just on diagonals
                static const std::array<direction, 4> possibilities = { { direction::EAST, direction::WEST, direction::NORTH, direction::SOUTH } };
                const direction extra_lava_dir = random_entry( possibilities );
                point extra;
                switch( extra_lava_dir ) {
                    case direction::NORTH:
                        extra.y = -1;
                        break;
                    case direction::EAST:
                        extra.x = 1;
                        break;
                    case direction::SOUTH:
                        extra.y = 1;
                        break;
                    case direction::WEST:
                        extra.x = -1;
                        break;
                    default:
                        break;
                }

                const tripoint portal_location = { p1 + tripoint( extra, abs_sub.z ) };
                m.add_field( portal_location, fd_fatigue, 3 );

                std::set<point> ignited;
                place_fumarole( m, p1, p2, ignited );
                place_fumarole( m, p1 + extra, p2 + extra,
                                ignited );

                for( const point &i : ignited ) {
                    // Don't need to do anything to tiles that already have lava on them
                    if( m.ter( i ) != t_lava ) {
                        // Spawn an intense but short-lived fire
                        // Any furniture or buildings will catch fire, otherwise it will burn out quickly
                        m.add_field( tripoint( i, abs_sub.z ), fd_fire, 15, 1_minutes );
                    }
                }
            }
            break;
        }
        //Anomaly caused by the portal and spawned an artifact
        case 6: {
            m.add_field( portal_location, fd_fatigue, 3 );
            artifact_natural_property prop =
                static_cast<artifact_natural_property>( rng( ARTPROP_NULL + 1, ARTPROP_MAX - 1 ) );
            m.create_anomaly( portal_location, prop );
            m.spawn_artifact( p + tripoint( rng( -1, 1 ), rng( -1, 1 ), abs_sub.z ),
                              relic_procgen_data_alien_reality );
            break;
        }
    }

    return true;
}

static bool mx_shia( map &m, const tripoint &loc )
{
    // A rare chance to spawn Shia. This was extracted from the hardcoded forest mapgen
    // and moved into a map extra, but it still has a one_in chance of spawning because
    // otherwise the extreme rarity of this event wildly skewed the values for all of the
    // other extras.
    if( one_in( 5000 ) ) {
        m.add_spawn( mon_shia, 1, { SEEX, SEEY, loc.z } );
        return true;
    }

    return false;
}

static bool mx_jabberwock( map &m, const tripoint &loc )
{
    // A rare chance to spawn a jabberwock. This was extracted from the hardcoded forest mapgen
    // and moved into a map extra. It still has a one_in chance of spawning because otherwise
    // the rarity skewed the values for all the other extras too much. I considered moving it
    // into the monster group, but again the hardcoded rarity it had in the forest mapgen was
    // not easily replicated there.
    if( one_in( 50 ) ) {
        m.add_spawn( mon_jabberwock, 1, { SEEX, SEEY, loc.z } );
        return true;
    }

    return false;
}

static bool mx_grove( map &m, const tripoint &abs_sub )
{
    // From wikipedia - The main meaning of "grove" is a group of trees that grow close together,
    // generally without many bushes or other plants underneath.

    // This map extra finds the first tree in the area, and then converts all trees, young trees,
    // and shrubs in the area into that type of tree.

    ter_id tree;
    bool found_tree = false;
    for( int i = 0; i < SEEX * 2; i++ ) {
        for( int j = 0; j < SEEY * 2; j++ ) {
            const tripoint location( i, j, abs_sub.z );
            if( m.has_flag_ter( ter_furn_flag::TFLAG_TREE, location ) ) {
                tree = m.ter( location );
                found_tree = true;
            }
        }
    }

    if( !found_tree ) {
        return false;
    }

    for( int i = 0; i < SEEX * 2; i++ ) {
        for( int j = 0; j < SEEY * 2; j++ ) {
            const tripoint location( i, j, abs_sub.z );
            if( m.has_flag_ter( ter_furn_flag::TFLAG_SHRUB, location ) ||
                m.has_flag_ter( ter_furn_flag::TFLAG_TREE, location ) ||
                m.has_flag_ter( ter_furn_flag::TFLAG_YOUNG, location ) ) {
                m.ter_set( location, tree );
            }
        }
    }

    return true;
}

static bool mx_shrubbery( map &m, const tripoint &abs_sub )
{
    // This map extra finds the first shrub in the area, and then converts all trees, young trees,
    // and shrubs in the area into that type of shrub.

    ter_id shrubbery;
    bool found_shrubbery = false;
    for( int i = 0; i < SEEX * 2; i++ ) {
        for( int j = 0; j < SEEY * 2; j++ ) {
            const tripoint location( i, j, abs_sub.z );
            if( m.has_flag_ter( ter_furn_flag::TFLAG_SHRUB, location ) ) {
                shrubbery = m.ter( location );
                found_shrubbery = true;
            }
        }
    }

    if( !found_shrubbery ) {
        return false;
    }

    for( int i = 0; i < SEEX * 2; i++ ) {
        for( int j = 0; j < SEEY * 2; j++ ) {
            const tripoint location( i, j, abs_sub.z );
            if( m.has_flag_ter( ter_furn_flag::TFLAG_SHRUB, location ) ||
                m.has_flag_ter( ter_furn_flag::TFLAG_TREE, location ) ||
                m.has_flag_ter( ter_furn_flag::TFLAG_YOUNG, location ) ) {
                m.ter_set( location, shrubbery );
            }
        }
    }

    return true;
}

static bool mx_clearcut( map &m, const tripoint &abs_sub )
{
    // From wikipedia - Clearcutting, clearfelling or clearcut logging is a forestry/logging
    // practice in which most or all trees in an area are uniformly cut down.

    // This map extra converts all trees and young trees in the area to stumps.

    ter_id stump( "t_stump" );

    bool did_something = false;

    for( int i = 0; i < SEEX * 2; i++ ) {
        for( int j = 0; j < SEEY * 2; j++ ) {
            const tripoint location( i, j, abs_sub.z );
            if( m.has_flag_ter( ter_furn_flag::TFLAG_TREE, location ) ||
                m.has_flag_ter( ter_furn_flag::TFLAG_YOUNG, location ) ) {
                if( !did_something ) {
                    did_something = true;
                }
                m.ter_set( location, stump );
            }
        }
    }

    return did_something;
}

static bool mx_pond( map &m, const tripoint &abs_sub )
{
    // This map extra creates small ponds using a simple cellular automaton.

    constexpr int width = SEEX * 2;
    constexpr int height = SEEY * 2;

    // Generate the cells for our lake.
    std::vector<std::vector<int>> current = CellularAutomata::generate_cellular_automaton( width,
                                            height, 55, 5, 4, 3 );

    // Loop through and turn every live cell into water.
    // Do a roll for our three possible lake types:
    // - all deep water
    // - all shallow water
    // - shallow water on the shore, deep water in the middle
    const int lake_type = rng( 1, 3 );
    for( int i = 0; i < width; i++ ) {
        for( int j = 0; j < height; j++ ) {
            if( current[i][j] == 1 ) {
                const tripoint location( i, j, abs_sub.z );
                m.furn_set( location, f_null );

                switch( lake_type ) {
                    case 1:
                        m.ter_set( location, t_water_sh );
                        break;
                    case 2:
                        m.ter_set( location, t_water_dp );
                        break;
                    case 3:
                        const int neighbors = CellularAutomata::neighbor_count( current, width, height, point( i, j ) );
                        if( neighbors == 8 ) {
                            m.ter_set( location, t_water_dp );
                        } else {
                            m.ter_set( location, t_water_sh );
                        }
                        break;
                }
            }
        }
    }

    m.place_spawns( GROUP_FISH, 1, point_zero, point( width, height ), 0.15f );

    return true;
}

static bool mx_clay_deposit( map &m, const tripoint &abs_sub )
{
    // This map extra creates small clay deposits using a simple cellular automaton.

    constexpr int width = SEEX * 2;
    constexpr int height = SEEY * 2;

    for( int tries = 0; tries < 5; tries++ ) {
        // Generate the cells for our clay deposit.
        std::vector<std::vector<int>> current = CellularAutomata::generate_cellular_automaton( width,
                                                height, 35, 5, 4, 3 );

        // With our settings for the CA, it's sometimes possible to get a bad generation with not enough
        // alive cells (or even 0).
        int alive_count = 0;
        for( int i = 0; i < width; i++ ) {
            for( int j = 0; j < height; j++ ) {
                alive_count += current[i][j];
            }
        }

        // If we have fewer than 4 alive cells, lets try again.
        if( alive_count < 4 ) {
            continue;
        }

        // Loop through and turn every live cell into clay.
        for( int i = 0; i < width; i++ ) {
            for( int j = 0; j < height; j++ ) {
                if( current[i][j] == 1 ) {
                    const tripoint location( i, j, abs_sub.z );
                    m.furn_set( location, f_null );
                    m.ter_set( location, t_clay );
                }
            }
        }

        // If we got here, it meant we had a successful try and can just break out of
        // our retry loop.
        return true;
    }

    return false;
}

static bool mx_dead_vegetation( map &m, const tripoint &abs_sub )
{
    // This map extra kills all plant life, creating area of desolation.
    // Possible result of acid rain / radiation / etc.,
    // but reason is not exposed (no rads, acid pools, etc.)

    for( int i = 0; i < SEEX * 2; i++ ) {
        for( int j = 0; j < SEEY * 2; j++ ) {
            const tripoint loc( i, j, abs_sub.z );

            dead_vegetation_parser( m, loc );
        }
    }

    return true;
}

static bool mx_point_dead_vegetation( map &m, const tripoint &abs_sub )
{
    // This map extra creates patch of dead vegetation using a simple cellular automaton.
    // Lesser version of mx_dead_vegetation

    constexpr int width = SEEX * 2;
    constexpr int height = SEEY * 2;

    // Generate the cells for dead vegetation.
    std::vector<std::vector<int>> current = CellularAutomata::generate_cellular_automaton( width,
                                            height, 55, 5, 4, 3 );

    for( int i = 0; i < width; i++ ) {
        for( int j = 0; j < height; j++ ) {
            if( current[i][j] == 1 ) {
                const tripoint loc( i, j, abs_sub.z );
                dead_vegetation_parser( m, loc );
            }
        }
    }

    return true;
}

static void burned_ground_parser( map &m, const tripoint &loc )
{
    const furn_t &fid = m.furn( loc ).obj();
    const ter_id tid = m.ter( loc );
    const ter_t &tr = tid.obj();

    VehicleList vehs = m.get_vehicles();
    std::vector<vehicle *> vehicles;
    std::vector<tripoint> points;
    for( wrapped_vehicle vehicle : vehs ) {
        vehicles.push_back( vehicle.v );
        std::set<tripoint> occupied = vehicle.v->get_points();
        for( const tripoint &t : occupied ) {
            points.push_back( t );
        }
    }
    for( vehicle *vrem : vehicles ) {
        m.destroy_vehicle( vrem );
    }
    for( const tripoint &tri : points ) {
        m.furn_set( tri, f_wreckage );
    }

    // grass is converted separately
    // this method is deliberate to allow adding new post-terrains
    // (TODO: expand this list when new destroyed terrain is added)
    static const std::map<ter_id, ter_str_id> dies_into {{
            {t_grass, ter_t_grass_dead},
            {t_grass_long, ter_t_grass_dead},
            {t_grass_tall, ter_t_grass_dead},
            {t_moss, ter_t_grass_dead},
            {t_fungus, ter_t_dirt},
            {t_grass_golf, ter_t_grass_dead},
            {t_grass_white, ter_t_grass_dead},
        }};

    const auto iter = dies_into.find( tid );
    if( iter != dies_into.end() ) {
        if( one_in( 6 ) ) {
            m.ter_set( loc, t_dirt );
            m.spawn_item( loc, itype_ash, 1, rng( 1, 5 ) );
        } else if( one_in( 10 ) ) {
            // do nothing, save some spots from fire
        } else {
            m.ter_set( loc, iter->second );
        }
    }

    // fungus cannot be destroyed by map::destroy so ths method is employed
    if( fid.has_flag( ter_furn_flag::TFLAG_FUNGUS ) ) {
        if( one_in( 5 ) ) {
            m.furn_set( loc, f_ash );
        }
    }
    if( tr.has_flag( ter_furn_flag::TFLAG_FUNGUS ) ) {
        m.ter_set( loc, t_dirt );
        if( one_in( 5 ) ) {
            m.spawn_item( loc, itype_ash, 1, rng( 1, 5 ) );
        }
    }
    // destruction of trees is not absolute
    if( tr.has_flag( ter_furn_flag::TFLAG_TREE ) ) {
        if( one_in( 4 ) ) {
            m.ter_set( loc, ter_t_trunk );
        } else if( one_in( 4 ) ) {
            m.ter_set( loc, ter_t_stump );
        } else if( one_in( 4 ) ) {
            m.ter_set( loc, ter_t_tree_dead );
        } else {
            m.ter_set( loc, ter_t_dirt );
            m.furn_set( loc, f_ash );
            m.spawn_item( loc, itype_ash, 1, rng( 1, 100 ) );
        }
        // everything else is destroyed, ash is added
    } else if( ter_furn_has_flag( tr, fid, ter_furn_flag::TFLAG_FLAMMABLE ) ||
               ter_furn_has_flag( tr, fid, ter_furn_flag::TFLAG_FLAMMABLE_HARD ) ) {
        while( m.is_bashable( loc ) ) { // one is not enough
            m.destroy( loc, true );
        }
        if( one_in( 5 ) && !tr.has_flag( ter_furn_flag::TFLAG_LIQUID ) ) {
            m.spawn_item( loc, itype_ash, 1, rng( 1, 10 ) );
        }
    } else if( ter_furn_has_flag( tr, fid, ter_furn_flag::TFLAG_FLAMMABLE_ASH ) ) {
        while( m.is_bashable( loc ) ) {
            m.destroy( loc, true );
        }
        if( !m.is_open_air( loc ) ) {
            m.furn_set( loc, f_ash );
            if( !tr.has_flag( ter_furn_flag::TFLAG_LIQUID ) ) {
                m.spawn_item( loc, itype_ash, 1, rng( 1, 100 ) );
            }
        }
    }

    // burn-away flammable items
    while( m.flammable_items_at( loc ) ) {
        map_stack stack = m.i_at( loc );
        for( auto it = stack.begin(); it != stack.end(); ) {
            if( it->flammable() ) {
                m.create_burnproducts( loc, *it, it->weight() );
                it = stack.erase( it );
            } else {
                it++;
            }
        }
    }
}

static bool mx_point_burned_ground( map &m, const tripoint &abs_sub )
{
    // This map extra creates patch of burned ground using a simple cellular automaton.
    // Lesser version of mx_burned_ground

    constexpr int width = SEEX * 2;
    constexpr int height = SEEY * 2;

    // Generate the cells for dead vegetation.
    std::vector<std::vector<int>> current = CellularAutomata::generate_cellular_automaton( width,
                                            height, 55, 5, 4, 3 );

    for( int i = 0; i < width; i++ ) {
        for( int j = 0; j < height; j++ ) {
            if( current[i][j] == 1 ) {
                const tripoint loc( i, j, abs_sub.z );
                burned_ground_parser( m, loc );
            }
        }
    }

    return true;
}

static bool mx_burned_ground( map &m, const tripoint &abs_sub )
{
    // This map extra simulates effects of extensive past fire event; it destroys most vegetation,
    // and flammable objects, swaps vehicles with wreckage, levels houses, scatters ash etc.

    for( int i = 0; i < SEEX * 2; i++ ) {
        for( int j = 0; j < SEEY * 2; j++ ) {
            const tripoint loc( i, j, abs_sub.z );
            burned_ground_parser( m, loc );
        }
    }
    VehicleList vehs = m.get_vehicles();
    std::vector<vehicle *> vehicles;
    std::vector<tripoint> points;
    for( wrapped_vehicle vehicle : vehs ) {
        vehicles.push_back( vehicle.v );
        std::set<tripoint> occupied = vehicle.v->get_points();
        for( const tripoint &t : occupied ) {
            points.push_back( t );
        }
    }
    for( vehicle *vrem : vehicles ) {
        m.destroy_vehicle( vrem );
    }
    for( const tripoint &tri : points ) {
        m.furn_set( tri, f_wreckage );
    }

    return true;
}

static bool mx_reed( map &m, const tripoint &abs_sub )
{
    // This map extra is for populating river banks, lake shores, etc. with
    // water vegetation

    // intensity consistent for whole overmap terrain bit
    // so vegetation is placed from one_in(1) = full overgrowth
    // to one_in(4) = 1/4 of terrain;
    int intensity = rng( 1, 4 );

    const auto near_water = [ &m ]( tripoint loc ) {

        for( const tripoint &p : m.points_in_radius( loc, 1 ) ) {
            if( p == loc ) {
                continue;
            }
            if( m.ter( p ) == t_water_moving_sh || m.ter( p ) == t_water_sh ||
                m.ter( p ) == t_water_moving_dp || m.ter( p ) == t_water_dp ) {
                return true;
            }
        }
        return false;
    };

    weighted_int_list<furn_id> vegetation;
    vegetation.add( f_cattails, 15 );
    vegetation.add( f_lotus, 5 );
    vegetation.add( f_lilypad, 1 );
    for( int i = 0; i < SEEX * 2; i++ ) {
        for( int j = 0; j < SEEY * 2; j++ ) {
            const tripoint loc( i, j, abs_sub.z );
            if( ( m.ter( loc ) == t_water_sh || m.ter( loc ) == t_water_moving_sh ) &&
                one_in( intensity ) ) {
                m.furn_set( loc, vegetation.pick()->id() );
            }
            // tall grass imitates reed
            if( ( m.ter( loc ) == t_dirt || m.ter( loc ) == t_grass ) &&
                one_in( near_water( loc ) ? intensity : 7 ) ) {
                m.ter_set( loc, t_grass_tall );
            }
        }
    }

    return true;
}

static bool mx_roadworks( map &m, const tripoint &abs_sub )
{
    // This map extra creates road works on NS & EW roads, including barricades (as barrier poles),
    // holes in the road, scattered soil, chance for heavy utility vehicles and some working
    // equipment in a box
    // (curved roads & intersections excluded, perhaps TODO)

    const tripoint_abs_omt abs_omt( sm_to_omt_copy( abs_sub ) );
    const oter_id &north = overmap_buffer.ter( abs_omt + point_north );
    const oter_id &south = overmap_buffer.ter( abs_omt + point_south );
    const oter_id &west = overmap_buffer.ter( abs_omt + point_west );
    const oter_id &east = overmap_buffer.ter( abs_omt + point_east );

    const bool road_at_north = north->get_type_id() == oter_type_road;
    const bool road_at_south = south->get_type_id() == oter_type_road;
    const bool road_at_west = west->get_type_id() == oter_type_road;
    const bool road_at_east = east->get_type_id() == oter_type_road;

    // defect types
    weighted_int_list<ter_id> road_defects;
    road_defects.add( t_pit_shallow, 15 );
    road_defects.add( t_dirt, 15 );
    road_defects.add( t_dirtmound, 15 );
    road_defects.add( t_pavement, 55 );
    const weighted_int_list<ter_id> defects = road_defects;

    // location holders
    point defects_from; // road defects square start
    point defects_to; // road defects square end
    point defects_centered; //  road defects centered
    point veh; // vehicle
    point equipment; // equipment

    // determine placement of effects
    if( road_at_north && road_at_south && !road_at_east && !road_at_west ) {
        if( one_in( 2 ) ) { // west side of the NS road
            // road barricade
            line_furn( &m, f_barricade_road, point( 4, 0 ), point( 11, 7 ) );
            line_furn( &m, f_barricade_road, point( 11, 8 ), point( 11, 15 ) );
            line_furn( &m, f_barricade_road, point( 11, 16 ), point( 4, 23 ) );
            // road defects
            defects_from = { 9, 7 };
            defects_to = { 4, 16 };
            defects_centered = { rng( 4, 7 ), rng( 10, 16 ) };
            // vehicle
            veh.x = rng( 4, 6 );
            veh.y = rng( 8, 10 );
            // equipment
            if( one_in( 2 ) ) {
                equipment.x = rng( 0, 4 );
                equipment.y = rng( 1, 2 );
            } else {
                equipment.x = rng( 0, 4 );
                equipment.y = rng( 21, 22 );
            }
        } else { // east side of the NS road
            // road barricade
            line_furn( &m, f_barricade_road, point( 19, 0 ), point( 12, 7 ) );
            line_furn( &m, f_barricade_road, point( 12, 8 ), point( 12, 15 ) );
            line_furn( &m, f_barricade_road, point( 12, 16 ), point( 19, 23 ) );
            // road defects
            defects_from = { 13, 7 };
            defects_to = { 19, 16 };
            defects_centered = { rng( 15, 18 ), rng( 10, 14 ) };
            // vehicle
            veh.x = rng( 15, 19 );
            veh.y = rng( 8, 10 );
            // equipment
            if( one_in( 2 ) ) {
                equipment.x = rng( 20, 24 );
                equipment.y = rng( 1, 2 );
            } else {
                equipment.x = rng( 20, 24 );
                equipment.y = rng( 21, 22 );
            }
        }
    } else if( road_at_west && road_at_east && !road_at_north && !road_at_south ) {
        if( one_in( 2 ) ) { // north side of the EW road
            // road barricade
            line_furn( &m, f_barricade_road, point( 0, 4 ), point( 7, 11 ) );
            line_furn( &m, f_barricade_road, point( 8, 11 ), point( 15, 11 ) );
            line_furn( &m, f_barricade_road, point( 16, 11 ), point( 23, 4 ) );
            // road defects
            defects_from = { 7, 9 };
            defects_to = { 16, 4 };
            defects_centered = { rng( 8, 14 ), rng( 3, 8 ) };
            // vehicle
            veh.x = rng( 6, 8 );
            veh.y = rng( 4, 8 );
            // equipment
            if( one_in( 2 ) ) {
                equipment.x = rng( 1, 2 );
                equipment.y = rng( 0, 4 );
            } else {
                equipment.x = rng( 21, 22 );
                equipment.y = rng( 0, 4 );
            }
        } else { // south side of the EW road
            // road barricade
            line_furn( &m, f_barricade_road, point( 0, 19 ), point( 7, 12 ) );
            line_furn( &m, f_barricade_road, point( 8, 12 ), point( 15, 12 ) );
            line_furn( &m, f_barricade_road, point( 16, 12 ), point( 23, 19 ) );
            // road defects
            defects_from = { 7, 13 };
            defects_to = { 16, 19 };
            defects_centered = { rng( 8, 14 ), rng( 14, 18 ) };
            // vehicle
            veh.x = rng( 6, 8 );
            veh.y = rng( 14, 18 );
            // equipment
            if( one_in( 2 ) ) {
                equipment.x = rng( 1, 2 );
                equipment.y = rng( 20, 24 );
            } else {
                equipment.x = rng( 21, 22 );
                equipment.y = rng( 20, 24 );
            }
        }
    } else if( road_at_north && road_at_east && !road_at_west && !road_at_south ) {
        // SW side of the N-E road curve
        // road barricade
        // NOLINTNEXTLINE(cata-use-named-point-constants)
        line_furn( &m, f_barricade_road, point( 1, 0 ), point( 11, 0 ) );
        line_furn( &m, f_barricade_road, point( 12, 0 ), point( 23, 10 ) );
        line_furn( &m, f_barricade_road, point( 23, 22 ), point( 23, 11 ) );
        // road defects
        switch( rng( 1, 3 ) ) {
            case 1:
                defects_from = { 9, 8 };
                defects_to = { 14, 3 };
                break;
            case 2:
                defects_from = { 12, 11 };
                defects_to = { 17, 6 };
                break;
            case 3:
                defects_from = { 16, 15 };
                defects_to = { 21, 10 };
                break;
        }
        defects_centered = { rng( 8, 14 ), rng( 8, 14 ) };
        // vehicle
        veh.x = rng( 7, 15 );
        veh.y = rng( 7, 15 );
        // equipment
        if( one_in( 2 ) ) {
            equipment.x = rng( 0, 1 );
            equipment.y = rng( 2, 23 );
        } else {
            equipment.x = rng( 0, 22 );
            equipment.y = rng( 22, 23 );
        }
    } else if( road_at_south && road_at_west && !road_at_east && !road_at_north ) {
        // NE side of the S-W road curve
        // road barricade
        line_furn( &m, f_barricade_road, point( 0, 4 ), point( 0, 12 ) );
        line_furn( &m, f_barricade_road, point( 1, 13 ), point( 11, 23 ) );
        line_furn( &m, f_barricade_road, point( 12, 23 ), point( 19, 23 ) );
        // road defects
        switch( rng( 1, 3 ) ) {
            case 1:
                defects_from = { 2, 7 };
                defects_to = { 7, 12 };
                break;
            case 2:
                defects_from = { 11, 22 };
                defects_to = { 17, 6 };
                break;
            case 3:
                defects_from = { 6, 17 };
                defects_to = { 11, 13 };
                break;
        }
        defects_centered = { rng( 8, 14 ), rng( 8, 14 ) };
        // vehicle
        veh.x = rng( 7, 15 );
        veh.y = rng( 7, 15 );
        // equipment
        if( one_in( 2 ) ) {
            equipment.x = rng( 0, 23 );
            equipment.y = rng( 0, 3 );
        } else {
            equipment.x = rng( 20, 23 );
            equipment.y = rng( 0, 23 );
        }
    } else if( road_at_north && road_at_west && !road_at_east && !road_at_south ) {
        // SE side of the W-N road curve
        // road barricade
        line_furn( &m, f_barricade_road, point( 0, 12 ), point( 0, 19 ) );
        line_furn( &m, f_barricade_road, point( 1, 11 ), point( 12, 0 ) );
        line_furn( &m, f_barricade_road, point( 13, 0 ), point( 19, 0 ) );
        // road defects
        switch( rng( 1, 3 ) ) {
            case 1:
                defects_from = { 11, 2 };
                defects_to = { 16, 7 };
                break;
            case 2:
                defects_from = { 5, 7 };
                defects_to = { 11, 11 };
                break;
            case 3:
                defects_from = { 1, 12 };
                defects_to = { 6, 17 };
                break;
        }

        defects_centered = { rng( 8, 14 ), rng( 8, 14 ) };
        // vehicle
        veh.x = rng( 9, 18 );
        veh.y = rng( 9, 18 );
        // equipment
        if( one_in( 2 ) ) {
            equipment.x = rng( 20, 23 );
            equipment.y = rng( 0, 23 );
        } else {
            equipment.x = rng( 0, 23 );
            equipment.y = rng( 20, 23 );
        }
    } else if( road_at_south && road_at_east && !road_at_west && !road_at_north ) {
        // NW side of the S-E road curve
        // road barricade
        line_furn( &m, f_barricade_road, point( 4, 23 ), point( 12, 23 ) );
        line_furn( &m, f_barricade_road, point( 13, 22 ), point( 22, 13 ) );
        line_furn( &m, f_barricade_road, point( 23, 4 ), point( 23, 12 ) );
        // road defects
        switch( rng( 1, 3 ) ) {
            case 1:
                defects_from = { 17, 7 };
                defects_to = { 22, 12 };
                break;
            case 2:
                defects_from = { 12, 12 };
                defects_to = { 17, 17 };
                break;
            case 3:
                defects_from = { 7, 17 };
                defects_to = { 12, 22 };
                break;
        }

        defects_centered = { rng( 10, 16 ), rng( 10, 16 ) };
        // vehicle
        veh.x = rng( 6, 15 );
        veh.y = rng( 6, 15 );
        // equipment
        if( one_in( 2 ) ) {
            equipment.x = rng( 0, 3 );
            equipment.y = rng( 0, 23 );
        } else {
            equipment.x = rng( 0, 23 );
            equipment.y = rng( 0, 3 );
        }
    } else {
        return false; // crossroads and strange roads - no generation, bail out
    }
    // road defects generator
    switch( rng( 1, 5 ) ) {
        case 1:
            square( &m, defects, defects_from,
                    defects_to );
            break;
        case 2:
            rough_circle( &m, t_pit_shallow, defects_centered, rng( 2, 4 ) );
            break;
        case 3:
            circle( &m, t_pit_shallow, defects_centered, rng( 2, 4 ) );
            break;
        case 4:
            rough_circle( &m, t_dirtmound, defects_centered, rng( 2, 4 ) );
            break;
        case 5:
            circle( &m, t_dirtmound, defects_centered, rng( 2, 4 ) );
            break;
    }
    // soil generator
    for( int i = 1; i <= 10; i++ ) {
        m.spawn_item( point( rng( defects_from.x, defects_to.x ),
                             rng( defects_from.y, defects_to.y ) ), itype_material_soil );
    }
    // vehicle placer
    switch( rng( 1, 6 ) ) {
        case 1:
            m.add_vehicle( vehicle_prototype_road_roller, veh, random_direction() );
            break;
        case 2:
            m.add_vehicle( vehicle_prototype_excavator, veh, random_direction() );
            break;
        case 3:
        case 4:
        case 5:
        case 6:
            break;
            // 3-6 empty to reduce chance of vehicles on site
    }
    // equipment placer
    if( one_in( 3 ) ) {
        m.furn_set( equipment, f_crate_c );
        m.place_items( Item_spawn_data_mine_equipment, 100, tripoint( equipment, 0 ),
                       tripoint( equipment, 0 ), true, calendar::start_of_cataclysm, 100 );
    }

    return true;
}

static bool mx_mayhem( map &m, const tripoint &abs_sub )
{
    switch( rng( 1, 3 ) ) {
        //Car accident resulted in a shootout with two victims
        case 1: {
            m.add_vehicle( vehicle_prototype_car, point( 18, 9 ), 270_degrees );
            m.add_vehicle( vehicle_prototype_4x4_car, point( 20, 5 ), 0_degrees );

            m.spawn_item( { 16, 10, abs_sub.z }, itype_shot_hull );
            m.add_corpse( { 16, 9, abs_sub.z } );
            m.add_field( { 16, 9, abs_sub.z }, fd_blood, rng( 1, 3 ) );

            for( const auto &loc : m.points_in_radius( { 16, 3, abs_sub.z }, 1 ) ) {
                if( one_in( 2 ) ) {
                    m.spawn_item( loc, itype_9mm_casing );
                }
            }

            m.add_splatter_trail( fd_blood, { 16, 3, abs_sub.z }, { 23, 1, abs_sub.z } );
            m.add_corpse( { 23, 1, abs_sub.z } );
            break;
        }
        //Some cocky moron with friends got dragged out of limo and shot down by a military
        case 2: {
            m.add_vehicle( vehicle_prototype_limousine, point( 18, 9 ), 270_degrees );

            m.add_corpse( { 16, 9, abs_sub.z } );
            m.add_corpse( { 16, 11, abs_sub.z } );
            m.add_corpse( { 16, 12, abs_sub.z } );

            m.add_splatter_trail( fd_blood, { 16, 8, abs_sub.z }, { 16, 12, abs_sub.z } );

            for( const auto &loc : m.points_in_radius( { 12, 11, abs_sub.z }, 2 ) ) {
                if( one_in( 3 ) ) {
                    m.spawn_item( loc, itype_223_casing );
                }
            }
            break;
        }
        //Some unfortunate stopped at the roadside to change tire, but was ambushed and killed
        case 3: {
            m.add_vehicle( vehicle_prototype_car, point( 18, 12 ), 270_degrees );

            m.add_field( { 16, 15, abs_sub.z }, fd_blood, rng( 1, 3 ) );

            m.spawn_item( { 16, 16, abs_sub.z }, itype_wheel, 1, 0, calendar::start_of_cataclysm, 4 );
            m.spawn_item( { 16, 16, abs_sub.z }, itype_tire_iron );

            if( one_in( 2 ) ) { //Unknown people killed and robbed the poor guy
                m.put_items_from_loc( Item_spawn_data_everyday_corpse, { 16, 15, abs_sub.z } );
                m.spawn_item( { 21, 15, abs_sub.z }, itype_shot_hull );
            } else { //Wolves charged to the poor guy...
                m.add_corpse( { 16, 15, abs_sub.z } );
                m.add_splatter_trail( fd_gibs_flesh, { 16, 13, abs_sub.z }, { 16, 16, abs_sub.z } );
                m.add_field( { 15, 15, abs_sub.z }, fd_gibs_flesh, rng( 1, 3 ) );

                for( const auto &loc : m.points_in_radius( { 16, 15, abs_sub.z }, 1 ) ) {
                    if( one_in( 2 ) ) {
                        m.spawn_item( loc, itype_9mm_casing );
                    }
                }

                const int max_wolves = rng( 1, 3 );
                item body = item::make_corpse( mon_wolf );
                if( one_in( 2 ) ) { //...from the north
                    for( int i = 0; i < max_wolves; i++ ) {
                        const auto &loc = m.points_in_radius( { 12, 12, abs_sub.z }, 3 );
                        const tripoint where = random_entry( loc );
                        m.add_item_or_charges( where, body );
                        m.add_field( where, fd_blood, rng( 1, 3 ) );
                    }
                } else { //...from the south
                    for( int i = 0; i < max_wolves; i++ ) {
                        const auto &loc = m.points_in_radius( { 12, 18, abs_sub.z }, 3 );
                        const tripoint where = random_entry( loc );
                        m.add_item_or_charges( where, body );
                        m.add_field( where, fd_blood, rng( 1, 3 ) );
                    }
                }
            }
            break;
        }
    }

    return true;
}

static bool mx_casings( map &m, const tripoint &abs_sub )
{
    const auto items = item_group::items_from( Item_spawn_data_ammo_casings, calendar::turn );

    switch( rng( 1, 4 ) ) {
        //Pile of random casings in random place
        case 1: {
            const tripoint location = { rng( 1, SEEX * 2 - 2 ), rng( 1, SEEY * 2 - 2 ), abs_sub.z };
            //Spawn casings
            for( const auto &loc : m.points_in_radius( location, rng( 1, 2 ) ) ) {
                if( one_in( 2 ) ) {
                    m.spawn_items( loc, items );
                }
            }
            //Spawn random trash in random place
            for( int i = 0; i < rng( 1, 3 ); i++ ) {
                const auto trash =
                    item_group::items_from( Item_spawn_data_map_extra_casings, calendar::turn );
                const tripoint trash_loc = random_entry( m.points_in_radius( { SEEX, SEEY, abs_sub.z }, 10 ) );
                m.spawn_items( trash_loc, trash );
            }
            //Spawn blood and bloody rag and sometimes trail of blood
            if( one_in( 2 ) ) {
                m.add_field( location, fd_blood, rng( 1, 3 ) );
                if( one_in( 2 ) ) {
                    const tripoint bloody_rag_loc = random_entry( m.points_in_radius( location, 3 ) );
                    m.spawn_item( bloody_rag_loc, itype_rag, 1, 0, calendar::start_of_cataclysm, 0, { json_flag_FILTHY } );
                }
                if( one_in( 2 ) ) {
                    m.add_splatter_trail( fd_blood, location,
                                          random_entry( m.points_in_radius( location, rng( 1, 4 ) ) ) );
                }
            }

            break;
        }
        //Entire battlefield filled with casings
        case 2: {
            //Spawn casings
            for( int i = 0; i < SEEX * 2; i++ ) {
                for( int j = 0; j < SEEY * 2; j++ ) {
                    if( one_in( 20 ) ) {
                        m.spawn_items( point( i, j ), items );
                    }
                }
            }
            const tripoint location = { SEEX, SEEY, abs_sub.z };
            //Spawn random trash in random place
            for( int i = 0; i < rng( 1, 3 ); i++ ) {
                const auto trash =
                    item_group::items_from( Item_spawn_data_map_extra_casings, calendar::turn );
                const tripoint trash_loc = random_entry( m.points_in_radius( location, 10 ) );
                m.spawn_items( trash_loc, trash );
            }
            //Spawn blood and bloody rag in random place
            if( one_in( 2 ) ) {
                const tripoint random_place = random_entry( m.points_in_radius( location, rng( 1, 10 ) ) );
                m.add_field( random_place, fd_blood, rng( 1, 3 ) );
                if( one_in( 2 ) ) {
                    const tripoint bloody_rag_loc = random_entry( m.points_in_radius( random_place, 3 ) );
                    m.spawn_item( bloody_rag_loc, itype_rag, 1, 0, calendar::start_of_cataclysm, 0, { json_flag_FILTHY } );
                }
            }
            break;
        }
        //Person moved and fired in some direction
        case 3: {
            //Spawn casings and blood trail along the direction of movement
            const tripoint from = { rng( 1, SEEX * 2 - 2 ), rng( 1, SEEY * 2 - 2 ), abs_sub.z };
            const tripoint to = { rng( 1, SEEX * 2 - 2 ), rng( 1, SEEY * 2 - 2 ), abs_sub.z };
            std::vector<tripoint> casings = line_to( from, to );
            for( auto &i : casings ) {
                if( one_in( 2 ) ) {
                    m.spawn_items( { i.xy(), abs_sub.z }, items );
                    if( one_in( 2 ) ) {
                        m.add_field( { i.xy(), abs_sub.z }, fd_blood, rng( 1, 3 ) );
                    }
                }
            }
            //Spawn random trash in random place
            for( int i = 0; i < rng( 1, 3 ); i++ ) {
                const auto trash =
                    item_group::items_from( Item_spawn_data_map_extra_casings, calendar::turn );
                const tripoint trash_loc =
                    random_entry( m.points_in_radius( { SEEX, SEEY, abs_sub.z }, 10 ) );
                m.spawn_items( trash_loc, trash );
            }
            //Spawn blood and bloody rag at the destination
            if( one_in( 2 ) ) {
                m.add_field( from, fd_blood, rng( 1, 3 ) );
                if( one_in( 2 ) ) {
                    const tripoint bloody_rag_loc = random_entry( m.points_in_radius( to, 3 ) );
                    m.spawn_item( bloody_rag_loc, itype_rag, 1, 0, calendar::start_of_cataclysm, 0, { json_flag_FILTHY } );
                }
            }
            break;
        }
        //Two persons shot and created two piles of casings
        case 4: {
            const tripoint first_loc = { rng( 1, SEEX - 2 ), rng( 1, SEEY - 2 ), abs_sub.z };
            const tripoint second_loc = { rng( 1, SEEX * 2 - 2 ), rng( 1, SEEY * 2 - 2 ), abs_sub.z };
            const auto first_items =
                item_group::items_from( Item_spawn_data_ammo_casings, calendar::turn );
            const auto second_items =
                item_group::items_from( Item_spawn_data_ammo_casings, calendar::turn );

            for( const auto &loc : m.points_in_radius( first_loc, rng( 1, 2 ) ) ) {
                if( one_in( 2 ) ) {
                    m.spawn_items( loc, first_items );
                }
            }
            for( const auto &loc : m.points_in_radius( second_loc, rng( 1, 2 ) ) ) {
                if( one_in( 2 ) ) {
                    m.spawn_items( loc, second_items );
                }
            }
            //Spawn random trash in random place
            for( int i = 0; i < rng( 1, 3 ); i++ ) {
                const auto trash =
                    item_group::items_from( Item_spawn_data_map_extra_casings, calendar::turn );
                const tripoint trash_loc = random_entry( m.points_in_radius( { SEEX, SEEY, abs_sub.z }, 10 ) );
                m.spawn_items( trash_loc, trash );
            }
            //Spawn blood and bloody rag at the first location, sometimes trail of blood
            if( one_in( 2 ) ) {
                m.add_field( first_loc, fd_blood, rng( 1, 3 ) );
                if( one_in( 2 ) ) {
                    const tripoint bloody_rag_loc = random_entry( m.points_in_radius( first_loc, 3 ) );
                    m.spawn_item( bloody_rag_loc, itype_rag, 1, 0, calendar::start_of_cataclysm, 0, { json_flag_FILTHY } );
                }
                if( one_in( 2 ) ) {
                    m.add_splatter_trail( fd_blood, first_loc,
                                          random_entry( m.points_in_radius( first_loc, rng( 1, 4 ) ) ) );
                }
            }
            //Spawn blood and bloody rag at the second location, sometimes trail of blood
            if( one_in( 2 ) ) {
                m.add_field( second_loc, fd_blood, rng( 1, 3 ) );
                if( one_in( 2 ) ) {
                    const tripoint bloody_rag_loc = random_entry( m.points_in_radius( second_loc, 3 ) );
                    m.spawn_item( bloody_rag_loc, itype_rag, 1, 0, calendar::start_of_cataclysm, 0, { json_flag_FILTHY } );
                }
                if( one_in( 2 ) ) {
                    m.add_splatter_trail( fd_blood, second_loc,
                                          random_entry( m.points_in_radius( second_loc, rng( 1, 4 ) ) ) );
                }
            }
            break;
        }
    }

    return true;
}

static bool mx_looters( map &m, const tripoint &abs_sub )
{
    const tripoint center( rng( 5, SEEX * 2 - 5 ), rng( 5, SEEY * 2 - 5 ), abs_sub.z );
    //25% chance to spawn a corpse with some blood around it
    if( one_in( 4 ) && m.passable( center ) ) {
        m.add_corpse( center );
        for( int i = 0; i < rng( 1, 3 ); i++ ) {
            m.add_field( random_entry( m.points_in_radius( center, 1 ) ), fd_blood, rng( 1, 3 ) );
        }
    }

    //Spawn up to 5 hostile bandits with equal chance to be ranged or melee type
    const int num_looters = rng( 1, 5 );
    for( int i = 0; i < num_looters; i++ ) {
        if( const cata::optional<tripoint> pos_ = random_point( m.points_in_radius( center, rng( 1,
        4 ) ), [&]( const tripoint & p ) {
        return m.passable( p );
        } ) ) {
            m.place_npc( pos_->xy(), string_id<npc_template>( one_in( 2 ) ? "thug" : "bandit" ) );
        }
    }

    return true;
}

static bool mx_corpses( map &m, const tripoint &abs_sub )
{
    const int num_corpses = rng( 1, 5 );
    //Spawn up to 5 human corpses in random places
    for( int i = 0; i < num_corpses; i++ ) {
        const tripoint corpse_location = { rng( 1, SEEX * 2 - 1 ), rng( 1, SEEY * 2 - 1 ), abs_sub.z };
        if( m.passable( corpse_location ) ) {
            m.add_field( corpse_location, fd_blood, rng( 1, 3 ) );
            m.put_items_from_loc( Item_spawn_data_everyday_corpse, corpse_location );
            //50% chance to spawn blood in every tile around every corpse in 1-tile radius
            for( const auto &loc : m.points_in_radius( corpse_location, 1 ) ) {
                if( one_in( 2 ) ) {
                    m.add_field( loc, fd_blood, rng( 1, 3 ) );
                }
            }
        }
    }
    //10% chance to spawn a flock of stray dogs feeding on human flesh
    if( one_in( 10 ) && num_corpses <= 4 ) {
        const tripoint corpse_location = { rng( 1, SEEX * 2 - 1 ), rng( 1, SEEY * 2 - 1 ), abs_sub.z };
        const std::vector<item> gibs =
            item_group::items_from( Item_spawn_data_remains_human_generic,
                                    calendar::start_of_cataclysm );
        m.spawn_items( corpse_location, gibs );
        m.add_field( corpse_location, fd_gibs_flesh, rng( 1, 3 ) );
        //50% chance to spawn gibs and dogs in every tile around what's left of human corpse in 1-tile radius
        for( const auto &loc : m.points_in_radius( corpse_location, 1 ) ) {
            if( one_in( 2 ) ) {
                m.add_field( { loc.xy(), abs_sub.z }, fd_gibs_flesh, rng( 1, 3 ) );
                m.place_spawns( GROUP_STRAY_DOGS, 1, loc.xy(), loc.xy(), 1, true );
            }
        }
    }

    return true;
}

static bool mx_city_trap( map &/*m*/, const tripoint &abs_sub )
{
    //First, find a city
    // TODO: fix point types
    const city_reference c = overmap_buffer.closest_city( tripoint_abs_sm( abs_sub ) );
    const tripoint_abs_omt city_center_omt =
        project_to<coords::omt>( c.abs_sm_pos );

    //Then fill vector with all roads inside the city radius
    std::vector<tripoint_abs_omt> valid_omt;
    for( const tripoint_abs_omt &p : points_in_radius( city_center_omt, c.city->size ) ) {
        if( overmap_buffer.check_ot( "road", ot_match_type::prefix, p ) ) {
            valid_omt.push_back( p );
        }
    }

    const tripoint_abs_omt road_omt = random_entry( valid_omt, city_center_omt );

    tinymap compmap;
    compmap.load( project_to<coords::sm>( road_omt ), false );

    const tripoint trap_center = { SEEX + rng( -5, 5 ), SEEY + rng( -5, 5 ), abs_sub.z };
    bool empty_3x3_square = false;

    //Then find an empty 3x3 pavement square (no other traps, furniture, or vehicles)
    for( const tripoint &p : points_in_radius( trap_center, 1 ) ) {
        if( ( compmap.ter( p ) == t_pavement || compmap.ter( p ) == t_pavement_y ) &&
            compmap.tr_at( p ).is_null() &&
            compmap.furn( p ) == f_null &&
            !compmap.veh_at( p ) ) {
            empty_3x3_square = true;
            break;
        }
    }

    //Finally, place a spinning blade trap...
    if( empty_3x3_square ) {
        for( const tripoint &p : points_in_radius( trap_center, 1 ) ) {
            compmap.trap_set( p, tr_blade );
        }
        compmap.trap_set( trap_center, tr_engine );
        //... and a loudspeaker to attract zombies
        compmap.add_spawn( mon_turret_speaker, 1, trap_center );
    }

    compmap.save();

    return true;
}

FunctionMap builtin_functions = {
    { map_extra_mx_null, mx_null },
    { map_extra_mx_crater, mx_crater },
    { map_extra_mx_roadworks, mx_roadworks },
    { map_extra_mx_mayhem, mx_mayhem },
    { map_extra_mx_roadblock, mx_roadblock },
    { map_extra_mx_bandits_block, mx_bandits_block },
    { map_extra_mx_minefield, mx_minefield },
    { map_extra_mx_supplydrop, mx_supplydrop },
    { map_extra_mx_military, mx_military },
    { map_extra_mx_helicopter, mx_helicopter },
    { map_extra_mx_portal_in, mx_portal_in },
    { map_extra_mx_house_spider, mx_house_spider },
    { map_extra_mx_house_wasp, mx_house_wasp },
    { map_extra_mx_shia, mx_shia },
    { map_extra_mx_jabberwock, mx_jabberwock },
    { map_extra_mx_grove, mx_grove },
    { map_extra_mx_shrubbery, mx_shrubbery },
    { map_extra_mx_clearcut, mx_clearcut },
    { map_extra_mx_pond, mx_pond },
    { map_extra_mx_clay_deposit, mx_clay_deposit },
    { map_extra_mx_dead_vegetation, mx_dead_vegetation },
    { map_extra_mx_point_dead_vegetation, mx_point_dead_vegetation },
    { map_extra_mx_burned_ground, mx_burned_ground },
    { map_extra_mx_point_burned_ground, mx_point_burned_ground },
    { map_extra_mx_casings, mx_casings },
    { map_extra_mx_looters, mx_looters },
    { map_extra_mx_corpses, mx_corpses },
    { map_extra_mx_city_trap, mx_city_trap },
    { map_extra_mx_reed, mx_reed }
};

map_extra_pointer get_function( const map_extra_id &name )
{
    const auto iter = builtin_functions.find( name );
    if( iter == builtin_functions.end() ) {
        debugmsg( "no built-in map extra function with id %s", name.str() );
        return nullptr;
    }
    return iter->second;
}

std::vector<map_extra_id> all_function_names;
std::vector<map_extra_id> get_all_function_names()
{
    return all_function_names;
}

void apply_function( const map_extra_id &id, map &m, const tripoint_abs_sm &abs_sub )
{
    bool applied_successfully = false;

    const map_extra &extra = id.obj();
    switch( extra.generator_method ) {
        case map_extra_method::map_extra_function: {
            const map_extra_pointer mx_func = get_function( map_extra_id( extra.generator_id ) );
            if( mx_func != nullptr ) {
                applied_successfully = mx_func( m, abs_sub.raw() );
            }
            break;
        }
        case map_extra_method::mapgen: {
            mapgendata dat( project_to<coords::omt>( abs_sub ), m, 0.0f, calendar::turn,
                            nullptr );
            applied_successfully = run_mapgen_func( extra.generator_id, dat );
            break;
        }
        case map_extra_method::update_mapgen: {
            mapgendata dat( project_to<coords::omt>( abs_sub ), m, 0.0f,
                            calendar::start_of_cataclysm, nullptr );
            applied_successfully =
                run_mapgen_update_func( update_mapgen_id( extra.generator_id ), dat );
            break;
        }
        case map_extra_method::null:
        default:
            break;
    }

    if( !applied_successfully ) {
        return;
    }

    overmap_buffer.add_extra( project_to<coords::omt>( abs_sub ), id );

    auto_notes::auto_note_settings &auto_note_settings = get_auto_notes_settings();

    // The player has discovered a map extra of this type.
    auto_note_settings.set_discovered( id );

    if( get_option<bool>( "AUTO_NOTES" ) && get_option<bool>( "AUTO_NOTES_MAP_EXTRAS" ) ) {

        // Only place note if the user has not disabled it via the auto note manager
        if( !auto_note_settings.has_auto_note_enabled( id, true ) ) {
            return;
        }

        const cata::optional<auto_notes::custom_symbol> &symbol =
            auto_note_settings.get_custom_symbol( extra.id );
        const std::string note_symbol = symbol ? ( *symbol ).get_symbol_string() : extra.get_symbol();
        const nc_color note_color = symbol ? ( *symbol ).get_color() : extra.color;
        const std::string mx_note =
            string_format( "%s:%s;<color_yellow>%s</color>: <color_white>%s</color>",
                           note_symbol,
                           get_note_string_from_color( note_color ),
                           extra.name(),
                           extra.description() );
        overmap_buffer.add_note( project_to<coords::omt>( abs_sub ), mx_note );
    }
}

FunctionMap all_functions()
{
    return builtin_functions;
}

void load( const JsonObject &jo, const std::string &src )
{
    extras.load( jo, src );
}

void check_consistency()
{
    extras.check();
}

void debug_spawn_test()
{
    uilist mx_menu;
    std::vector<std::string> mx_names;
    for( std::pair<const std::string, map_extras> &region_extra :
         region_settings_map["default"].region_extras ) {
        mx_menu.addentry( -1, true, -1, region_extra.first );
        mx_names.push_back( region_extra.first );
    }

    mx_menu.text = _( "Test which map extra list?" );
    while( true ) {
        mx_menu.query();
        const int index = mx_menu.ret;
        if( index >= static_cast<int>( mx_names.size() ) || index < 0 ) {
            break;
        }

        std::map<map_extra_id, int> results;
        map_extra_id mx_null = map_extra_id::NULL_ID();

        for( size_t a = 0; a < 32400; a++ ) {
            map_extras ex = region_settings_map["default"].region_extras[mx_names[index]];
            if( ex.chance > 0 && one_in( ex.chance ) ) {
                map_extra_id *extra = ex.values.pick();
                if( extra == nullptr ) {
                    results[mx_null]++;
                } else {
                    results[*ex.values.pick()]++;
                }
            } else {
                results[mx_null]++;
            }
        }

        std::multimap<int, map_extra_id> sorted_results;
        for( std::pair<const map_extra_id, int> &e : results ) {
            sorted_results.emplace( e.second, e.first );
        }
        uilist results_menu;
        results_menu.text = _( "Result of 32400 selections:" );
        for( std::pair<const int, map_extra_id> &r : sorted_results ) {
            results_menu.entries.emplace_back(
                static_cast<int>( results_menu.entries.size() ), true, -2,
                string_format( "%d x %s", r.first, r.second.str() ) );
        }
        results_menu.query();
    }
}

} // namespace MapExtras

bool map_extra::is_valid_for( const mapgendata &md ) const
{
    int z = md.zlevel();
    if( min_max_zlevel_ ) {
        if( z < min_max_zlevel_->first || z > min_max_zlevel_->second ) {
            return false;
        }
    }

    return true;
}

void map_extra::load( const JsonObject &jo, const std::string & )
{
    mandatory( jo, was_loaded, "name", name_ );
    mandatory( jo, was_loaded, "description", description_ );
    if( jo.has_object( "generator" ) ) {
        JsonObject jg = jo.get_object( "generator" );
        generator_method = jg.get_enum_value<map_extra_method>( "generator_method",
                           map_extra_method::null );
        mandatory( jg, was_loaded, "generator_id", generator_id );
    }
    optional( jo, was_loaded, "sym", symbol, unicode_codepoint_from_symbol_reader, NULL_UNICODE );
    color = jo.has_member( "color" ) ? color_from_string( jo.get_string( "color" ) ) : c_white;
    optional( jo, was_loaded, "autonote", autonote, false );
    optional( jo, was_loaded, "min_max_zlevel", min_max_zlevel_ );
}

void map_extra::check() const
{
    switch( generator_method ) {
        case map_extra_method::map_extra_function: {
            const map_extra_pointer mx_func = MapExtras::get_function( map_extra_id( generator_id ) );
            if( mx_func == nullptr ) {
                debugmsg( "invalid map extra function (%s) defined for map extra (%s)", generator_id, id.str() );
                break;
            }
            MapExtras::all_function_names.push_back( id );
            break;
        }
        case map_extra_method::mapgen: {
            MapExtras::all_function_names.push_back( id );
            break;
        }
        case map_extra_method::update_mapgen: {
            const auto update_mapgen_func = update_mapgens.find( update_mapgen_id( generator_id ) );
            if( update_mapgen_func == update_mapgens.end() ||
                update_mapgen_func->second.funcs().empty() ) {
                debugmsg( "invalid update mapgen function (%s) defined for map extra (%s)", generator_id,
                          id.str() );
                break;
            }
            MapExtras::all_function_names.push_back( id );
            break;
        }
        case map_extra_method::null:
        default:
            break;
    }
}
