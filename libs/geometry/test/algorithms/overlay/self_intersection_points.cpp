// Boost.Geometry (aka GGL, Generic Geometry Library)
// Unit Test

// Copyright (c) 2007-2012 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2008-2012 Bruno Lalande, Paris, France.
// Copyright (c) 2009-2012 Mateusz Loskot, London, UK.

// Parts of Boost.Geometry are redesigned from Geodan's Geographic Library
// (geolib/GGL), copyright (c) 1995-2010 Geodan, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <iomanip>
#include <string>

#include <geometry_test_common.hpp>


#include <boost/geometry/algorithms/correct.hpp>
#include <boost/geometry/algorithms/intersection.hpp>
#include <boost/geometry/algorithms/intersects.hpp>
//#include <boost/geometry/algorithms/detail/overlay/self_intersection_points.hpp>
#include <boost/geometry/algorithms/detail/overlay/self_turn_points.hpp>

#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/linestring.hpp>
#include <boost/geometry/geometries/polygon.hpp>

#include <boost/geometry/strategies/strategies.hpp>

#include <boost/geometry/io/wkt/read.hpp>
#include <boost/geometry/io/wkt/write.hpp>

#include <algorithms/overlay/overlay_cases.hpp>


#if defined(TEST_WITH_SVG)
#  include <boost/geometry/io/svg/svg_mapper.hpp>
#endif


template <typename Geometry>
static void test_self_intersection_points(std::string const& case_id,
            int expected_count,
            Geometry const& geometry,
            bool check_has_intersections,
            double precision = 0.001)
{
    typedef bg::detail::overlay::turn_info
    <
        typename bg::point_type<Geometry>::type
    > turn_info;

    std::vector<turn_info> turns;

    ///bg::get_intersection_points(geometry, turns);

    bg::detail::self_get_turn_points::no_interrupt_policy policy;
    bg::self_turns
        <
            bg::detail::overlay::assign_null_policy
        >(geometry, turns, policy);


    typedef typename bg::coordinate_type<Geometry>::type ct;
    ct zero = ct();
    ct x = zero, y = zero;
    BOOST_FOREACH(turn_info const& turn, turns)
    {
        x += bg::get<0>(turn.point);
        y += bg::get<1>(turn.point);
    }
    int n = boost::size(turns);
    if (n > 0)
    {
        x /= n;
        y /= n;
    }

    BOOST_CHECK_EQUAL(expected_count, n);

    if (expected_count > 0)
    {
        BOOST_CHECK_EQUAL(bg::intersects(geometry), true);

        if (check_has_intersections)
        {
            try
            {
                boost::geometry::detail::overlay::has_self_intersections(geometry);
                BOOST_CHECK_MESSAGE(false, "Case " << case_id << " there are no self-intersections detected!"); 
            }
            catch(...)
            {
            }
        }
    }
    else
    {
        if (check_has_intersections)
        {
            try
            {
                boost::geometry::detail::overlay::has_self_intersections(geometry);
            }
            catch(...)
            {
                BOOST_CHECK_MESSAGE(false, "Case " << case_id << " there are self-intersections detected!"); 
            }
        }
    }


#if defined(TEST_WITH_SVG)
    {
        std::ostringstream filename;
        filename << "self_ip" << case_id << ".svg";

        std::ofstream svg(filename.str().c_str());

        bg::svg_mapper<typename bg::point_type<Geometry>::type> mapper(svg, 500, 500);
        mapper.add(geometry);

        mapper.map(geometry, "fill:rgb(255,255,128);stroke:rgb(0,0,0);stroke-width:1");

        BOOST_FOREACH(turn_info const& turn, turns)
        {
            mapper.map(turn.point, "fill:rgb(255,128,0);stroke:rgb(0,0,100);stroke-width:1");
        }
    }
#endif
}


template <typename Geometry, typename T>
void test_self_overlay(std::string const& case_id, T const& expected,
            std::string const& wkt,
            bool check_has_intersections = true)
{
    Geometry geometry;
    bg::read_wkt(wkt, geometry);
    bg::correct(geometry);

#ifdef GEOMETRY_DEBUG_INTERSECTION
    std::cout << std::endl << std::endl << "# " << case_id << std::endl;
#endif
    test_self_intersection_points(case_id, expected, geometry, check_has_intersections);
}



template <typename P>
void test_self_all()
{
    typedef bg::model::polygon<P> polygon;
    typedef bg::model::linestring<P> linestring;

    // Just a normal polygon
    test_self_overlay<polygon>("1", 0,
            "POLYGON((0 0,0 4,1.5 2.5,2.5 1.5,4 0,0 0))");

    // TODO: clean-up and visualize testcases
        
    // Self intersecting
    test_self_overlay<polygon>("2", 2,
            "POLYGON((1 2,1 1,2 1,2 2.25,3 2.25,3 0,0 0,0 3,3 3,2.75 2,1 2))");

    // Self intersecting in last segment
    test_self_overlay<polygon>("3", 1,
            "POLYGON((0 2,2 4,2 0,4 2,0 2))");

    // Self tangent
    test_self_overlay<polygon>("4", 1,
            "POLYGON((0 0,0 4,4 4,4 0,2 4,0 0))", false);

    // Self tangent in corner
    test_self_overlay<polygon>("5", 1,
            "POLYGON((0 0,0 4,4 4,4 0,0 4,2 0,0 0))", false);

    // With spike
    test_self_overlay<polygon>("6", 1,
            "POLYGON((0 0,0 4,4 4,4 2,6 2,4 2,4 0,0 0))");

    // Non intersection, but with duplicate
    test_self_overlay<polygon>("d1", 0,
            "POLYGON((0 0,0 4,4 0,4 0,0 0))");

    // With many duplicates
    test_self_overlay<polygon>("d2", 0,
            "POLYGON((0 0,0 1,0 1,0 1,0 2,0 2,0 3,0 3,0 3,0 3,0 4,2 4,2 4,4 4,4 0,4 0,3 0,3 0,3 0,3 0,3 0,0 0))");

    // Hole: interior tangent to exterior
    test_self_overlay<polygon>("h1", 1,
            "POLYGON((0 0,0 4,4 4,4 0,0 0),(1 2,2 4,3 2,1 2))", false);

    // Hole: interior intersecting exterior
    test_self_overlay<polygon>("h2", 2,
            "POLYGON((0 0,0 4,4 4,4 0,0 0),(1 1,1 3,5 4,1 1))");

    // Hole: two intersecting holes
    test_self_overlay<polygon>("h3", 2,
            "POLYGON((0 0,0 4,4 4,4 0,0 0),(1 1,1 3,3 3,3 1,1 1),(2 2,2 3.5,3.5 3.5,3.5 2,2 2))");

    // Hole: self-intersecting hole
    test_self_overlay<polygon>("h4", 2,
            "POLYGON((0 0,0 4,4 4,4 0,0 0),(1 1,3 3,3 2.5,1 3.5,1.5 3.5,1 1))");

    // Many loops (e.g. as result from buffering)
    test_self_overlay<polygon>("case_1", 6,
            "POLYGON((1 3,0 9,9 5,1 7,9 8,2 5,10 10,9 2,1 3))");

    test_self_overlay<polygon>("buffer_poly_indentation8_d_r", 3,
            "POLYGON((-0.8 5,-0.799036 5.03925,-0.796148 5.07841,-0.791341 5.11738,-0.784628 5.15607,-0.776025 5.19438,-0.765552 5.23223,-0.753235 5.26951,-0.739104 5.30615,-0.723191 5.34204,-0.705537 5.37712,-0.686183 5.41128,-0.665176 5.44446,-0.642566 5.47656,-0.618408 5.50751,-0.592761 5.53725,-0.537247 5.59276,-0.507515 5.61841,-0.476559 5.64257,-0.444456 5.66518,-0.411282 5.68618,-0.377117 5.70554,-0.342044 5.72319,-0.306147 5.7391,-0.269512 5.75324,-0.232228 5.76555,-0.194384 5.77603,-0.156072 5.78463,-0.117384 5.79134,-0.0784137 5.79615,-0.0392541 5.79904,0 5.8,4 5.8,4.03925 5.79904,4.07841 5.79615,4.11738 5.79134,4.15607 5.78463,4.19438 5.77603,4.23223 5.76555,4.26951 5.75324,4.30615 5.7391,4.34204 5.72319,4.37712 5.70554,4.41128 5.68618,4.44446 5.66518,4.47656 5.64257,4.50751 5.61841,4.53725 5.59276,4.59276 5.53725,4.61841 5.50751,4.64257 5.47656,4.66518 5.44446,4.68618 5.41128,4.70554 5.37712,4.72319 5.34204,4.7391 5.30615,4.75324 5.26951,4.76555 5.23223,4.77603 5.19438,4.78463 5.15607,4.79134 5.11738,4.79615 5.07841,4.79904 5.03925,4.8 5,4.8 4,4.79976 3.98037,4.79904 3.96075,4.79783 3.94115,4.79615 3.92159,4.79398 3.90207,4.79134 3.88262,4.78822 3.86323,4.78463 3.84393,4.78056 3.82472,4.77603 3.80562,4.77102 3.78663,4.76555 3.76777,4.75962 3.74905,4.75324 3.73049,4.74639 3.71208,4.73137 3.67581,4.72319 3.65796,4.71458 3.64031,4.70554 3.62288,4.69607 3.60568,4.68618 3.58872,4.67588 3.572,4.66518 3.55554,4.65407 3.53935,4.64257 3.52344,4.63068 3.50781,4.61841 3.49249,4.60577 3.47746,4.59276 3.46275,4.5794 3.44837,4.56569 3.43431,3.56569 2.43431,3.53725 2.40724,3.50751 2.38159,3.47656 2.35743,3.44446 2.33482,3.41128 2.31382,3.37712 2.29446,3.34204 2.27681,3.30615 2.2609,3.26951 2.24676,3.23223 2.23445,3.19438 2.22397,3.15607 2.21537,3.11738 2.20866,3.07841 2.20385,3.03925 2.20096,2.96075 2.20096,2.92159 2.20385,2.88262 2.20866,2.84393 2.21537,2.80562 2.22397,2.76777 2.23445,2.73049 2.24676,2.69385 2.2609,2.65796 2.27681,2.62288 2.29446,2.58872 2.31382,2.55554 2.33482,2.52344 2.35743,2.49249 2.38159,2.46275 2.40724,2.43431 2.43431,2.8 2.06863,2.8 2.93137,2.43431 2.56569,2.46275 2.59276,2.49249 2.61841,2.52344 2.64257,2.55554 2.66518,2.58872 2.68618,2.62288 2.70554,2.65796 2.72319,2.69385 2.7391,2.73049 2.75324,2.76777 2.76555,2.80562 2.77603,2.84393 2.78463,2.88262 2.79134,2.92159 2.79615,2.96075 2.79904,3.03925 2.79904,3.07841 2.79615,3.11738 2.79134,3.15607 2.78463,3.19438 2.77603,3.23223 2.76555,3.26951 2.75324,3.30615 2.7391,3.34204 2.72319,3.37712 2.70554,3.41128 2.68618,3.44446 2.66518,3.47656 2.64257,3.50751 2.61841,3.53725 2.59276,3.56569 2.56569,4.56569 1.56569,4.5794 1.55163,4.59276 1.53725,4.60577 1.52254,4.61841 1.50751,4.63068 1.49219,4.64257 1.47656,4.65407 1.46065,4.66518 1.44446,4.67588 1.428,4.68618 1.41128,4.69607 1.39432,4.70554 1.37712,4.71458 1.35969,4.72319 1.34204,4.73137 1.32419,4.74639 1.28792,4.75324 1.26951,4.75962 1.25095,4.76555 1.23223,4.77102 1.21337,4.77603 1.19438,4.78056 1.17528,4.78463 1.15607,4.78822 1.13677,4.79134 1.11738,4.79398 1.09793,4.79615 1.07841,4.79783 1.05885,4.79904 1.03925,4.79976 1.01963,4.8 1,4.8 0,4.79904 -0.0392541,4.79615 -0.0784137,4.79134 -0.117384,4.78463 -0.156072,4.77603 -0.194384,4.76555 -0.232228,4.75324 -0.269512,4.7391 -0.306147,4.72319 -0.342044,4.70554 -0.377117,4.68618 -0.411282,4.66518 -0.444456,4.64257 -0.476559,4.61841 -0.507515,4.59276 -0.537247,4.53725 -0.592761,4.50751 -0.618408,4.47656 -0.642566,4.44446 -0.665176,4.41128 -0.686183,4.37712 -0.705537,4.34204 -0.723191,4.30615 -0.739104,4.26951 -0.753235,4.23223 -0.765552,4.19438 -0.776025,4.15607 -0.784628,4.11738 -0.791341,4.07841 -0.796148,4.03925 -0.799036,4 -0.8,0 -0.8,-0.0392541 -0.799036,-0.0784137 -0.796148,-0.117384 -0.791341,-0.156072 -0.784628,-0.194384 -0.776025,-0.232228 -0.765552,-0.269512 -0.753235,-0.306147 -0.739104,-0.342044 -0.723191,-0.377117 -0.705537,-0.411282 -0.686183,-0.444456 -0.665176,-0.476559 -0.642566,-0.507515 -0.618408,-0.537247 -0.592761,-0.592761 -0.537247,-0.618408 -0.507515,-0.642566 -0.476559,-0.665176 -0.444456,-0.686183 -0.411282,-0.705537 -0.377117,-0.723191 -0.342044,-0.739104 -0.306147,-0.753235 -0.269512,-0.765552 -0.232228,-0.776025 -0.194384,-0.784628 -0.156072,-0.791341 -0.117384,-0.796148 -0.0784137,-0.799036 -0.0392541,-0.8 0,-0.8 5))");

    test_self_overlay<polygon>("toolkit_2", 12,
            "POLYGON((170718 605997,170718 605997,170776 606016,170773 606015,170786 606020,170778 606016,170787 606021,170781 606017,170795 606028,170795 606028,170829 606055,170939 606140,170933 605968,170933 605968,170932 605908,170929 605834,170920 605866,170961 605803,170739 605684,170699 605749,170691 605766,170693 605762,170686 605775,170688 605771,170673 605794,170676 605790,170668 605800,170672 605796,170651 605818,170653 605816,170639 605829,170568 605899,170662 605943,170633 605875,170603 605961,170718 605997))");

    // Real-life

    std::string const ticket17 = "POLYGON ((-122.28139163 37.37319149,-122.28100699 37.37273669,-122.28002186 37.37303123,-122.27979681 37.37290072,-122.28007349 37.37240493,-122.27977334 37.37220360,-122.27819720 37.37288580,-122.27714184 37.37275161,-122.27678628 37.37253167,-122.27766437 37.37180973,-122.27804382 37.37121453,-122.27687664 37.37101354,-122.27645829 37.37203386,-122.27604423 37.37249110,-122.27632234 37.37343339,-122.27760980 37.37391082,-122.27812478 37.37800320,-122.26117222 37.39121007,-122.25572289 37.39566631,-122.25547269 37.39564971,-122.25366304 37.39552993,-122.24919976 37.39580268,-122.24417933 37.39366907,-122.24051443 37.39094143,-122.23246277 37.38100418,-122.23606766 37.38141338,-122.24001587 37.37738940,-122.23666848 37.37609347,-122.23057450 37.37882170,-122.22679803 37.37807143,-122.22525727 37.37448817,-122.22523229 37.37443000,-122.23083199 37.37609347,-122.23033486 37.37777891,-122.23169030 37.37732117,-122.23229178 37.37709687,-122.23237761 37.37631249,-122.23297776 37.37438834,-122.23872850 37.37165986,-122.24044511 37.36934068,-122.24671067 37.36865847,-122.24825570 37.36981819,-122.25151719 37.36947713,-122.25357721 37.36756706,-122.26001451 37.36579354,-122.25615213 37.36545239,-122.25486458 37.36245083,-122.25357721 37.36108651,-122.25194642 37.36013139,-122.24885652 37.35958557,-122.24911401 37.35849399,-122.25357721 37.35808470,-122.25675286 37.35897159,-122.25855539 37.35753887,-122.26181687 37.35828939,-122.26713837 37.35897159,-122.26782510 37.36108651,-122.26662339 37.36456559,-122.27288911 37.36722601,-122.27366159 37.36531602,-122.27168740 37.36470213,-122.27391900 37.36374701,-122.27074326 37.36245083,-122.27134408 37.35951742,-122.27426240 37.36135926,-122.27709482 37.36115474,-122.27966974 37.36231438,-122.27958391 37.36463382,-122.27572152 37.36463382,-122.27563569 37.36524779,-122.27700899 37.36593000,-122.27709482 37.36763529,-122.27554978 37.36838573,-122.27667254 37.36931478,-122.27677932 37.36932073,-122.27769362 37.36853987,-122.27942490 37.36830803,-122.28178776 37.36677917,-122.28509559 37.36443500,-122.28845129 37.36413744,-122.29194403 37.36695946,-122.29382577 37.36726817,-122.29600414 37.36898512,-122.29733083 37.36995398,-122.29593239 37.37141436,-122.29416649 37.37075898,-122.29325026 37.37108436,-122.29652910 37.37311697,-122.29584237 37.37374461,-122.29537583 37.37573372,-122.29487677 37.37752502,-122.30923212 37.37593011,-122.31122484 37.38230086,-122.31467994 37.38092472,-122.31715663 37.38252181,-122.32307970 37.38166978,-122.31985618 37.37667694,-122.32210304 37.37580220,-122.32581446 37.37589532,-122.32401730 37.37331839,-122.32960417 37.37189020,-122.33465527 37.37331906,-122.33425328 37.37623680,-122.33620676 37.37726132,-122.33397986 37.37822382,-122.33358918 37.38036590,-122.33202637 37.37986918,-122.33147954 37.38101784,-122.33394080 37.38198017,-122.33545239 37.38587943,-122.33478058 37.38785697,-122.33386050 37.38723721,-122.33350041 37.38571137,-122.33122003 37.38548891,-122.33140008 37.38650606,-122.33366042 37.38817490,-122.33244019 37.39157602,-122.33298157 37.39419201,-122.33164013 37.39477028,-122.33202017 37.39518351,-122.33358038 37.39499282,-122.33376050 37.39597811,-122.33550067 37.39734478,-122.33556069 37.39481797,-122.33344040 37.39292676,-122.33638094 37.38892189,-122.34240644 37.38852719,-122.34906293 37.38726898,-122.35072321 37.39338769,-122.34910291 37.39445252,-122.34796272 37.39410291,-122.34449043 37.39640534,-122.34500223 37.39729709,-122.34936291 37.39670910,-122.35098322 37.39531066,-122.35364623 37.39554510,-122.35434369 37.39612111,-122.35798429 37.39600988,-122.35768430 37.39478621,-122.36334519 37.39206871,-122.36604726 37.39203267,-122.36778592 37.39335592,-122.36518870 37.40022011,-122.36554552 37.40247752,-122.36370519 37.40331974,-122.36270506 37.40530591,-122.36320512 37.40670418,-122.36149849 37.40851392,-122.36730580 37.41054938,-122.37263720 37.41378932,-122.37161871 37.42076600,-122.36566153 37.42006292,-122.36520547 37.42742106,-122.37165953 37.43661157,-122.35943972 37.44459022,-122.35356359 37.44600810,-122.33792254 37.45796329,-122.35228518 37.47478091,-122.35127080 37.48181199,-122.34867342 37.48487322,-122.34359717 37.48801082,-122.33388431 37.48677650,-122.33142321 37.48429747,-122.32929580 37.48473149,-122.32609609 37.48291144,-122.32344850 37.48228229,-122.31924364 37.48410234,-122.31677299 37.48114051,-122.31431751 37.47848973,-122.31259201 37.47682190,-122.31515972 37.47568196,-122.31691389 37.47360309,-122.31292494 37.46960081,-122.31130153 37.46937743,-122.30889894 37.47124987,-122.30612839 37.47011613,-122.30149630 37.46568378,-122.30064277 37.46363784,-122.29283821 37.45922376,-122.28630141 37.45415497,-122.28883099 37.44629920,-122.28316717 37.44197138,-122.27554148 37.42297597,-122.25597410 37.40553692,-122.25196579 37.40129593,-122.25012043 37.40049143,-122.24823207 37.39897758,-122.24754551 37.39740941,-122.24778582 37.39621607,-122.24934787 37.39599102,-122.25005170 37.39871849,-122.25222328 37.39863668,-122.25342491 37.39737529,-122.25520162 37.39667289,-122.25528737 37.39522726,-122.27747460 37.37809616,-122.27977493 37.37858717,-122.28157729 37.37920106,-122.28322534 37.37952846,-122.28416939 37.38092656,-122.28621223 37.37984219,-122.28638389 37.37613857,-122.28382607 37.37843722,-122.27930278 37.37718220,-122.28196361 37.37652740,-122.28295058 37.37568167,-122.28216101 37.37523148,-122.28114822 37.37543608,-122.27934569 37.37528613,-122.27996369 37.37448121,-122.28104521 37.37454944,-122.28185197 37.37422883,-122.28290767 37.37474038,-122.28376597 37.37467224,-122.28428104 37.37399012,-122.28402346 37.37338989,-122.28610922 37.37364914,-122.28651264 37.37327388,-122.28672722 37.37207343,-122.28628398 37.37205448,-122.28574460 37.37166682,-122.28479711 37.37200981,-122.28327731 37.37137228,-122.28285511 37.37100700,-122.28279409 37.37125669,-122.28315527 37.37173756,-122.28321872 37.37220569,-122.28187007 37.37231918,-122.28193109 37.37294908,-122.28139163 37.37319149))";
    test_self_overlay<polygon>("ticket17", 2, ticket17);

    std::string const uscounty_ne = "POLYGON((-72.297814 44.183545,-72.30498 44.183149,-72.306552 44.183668,-72.321587 44.188629,-72.368353 44.204057,-72.362972 44.211115,-72.357354 44.218485,-72.336176 44.246264,-72.333283 44.25006,-72.329737 44.254806,-72.327061 44.258388,-72.316301 44.272583,-72.317394 44.293961,-72.317455 44.295412,-72.317621 44.298382,-72.31757 44.298423,-72.308843 44.30567,-72.308336 44.306161,-72.305947 44.30843,-72.295321 44.31908,-72.292014 44.321754,-72.289771 44.323567,-72.283878 44.32772,-72.282024 44.32927,-72.275139 44.335778,-72.271225 44.339322,-72.269779 44.340275,-72.269584 44.340461,-72.267563 44.343747,-72.255707 44.359532,-72.254273 44.361075,-72.249662 44.367162,-72.249549 44.367339,-72.24832 44.369514,-72.247963 44.369843,-72.245885 44.372717,-72.244210 44.375223,-72.240219 44.380321,-72.24014 44.380422,-72.229086 44.391162,-72.224592 44.39553,-72.224364 44.395751,-72.224549 44.395985,-72.226261 44.398726,-72.225478 44.399979,-72.223603 44.402875,-72.222984 44.403317,-72.222454 44.403753,-72.217109 44.411083,-72.218322 44.411745,-72.218833 44.412134,-72.218904 44.412313,-72.219023 44.412613,-72.219444 44.412803,-72.21963 44.412887,-72.220395 44.413162,-72.220619 44.413527,-72.22215 44.414167,-72.222276 44.414327,-72.222276 44.414784,-72.222565 44.415058,-72.22266 44.415469,-72.222565 44.415583,-72.22282 44.416063,-72.222757 44.416452,-72.222543 44.416596,-72.222247 44.416792,-72.221852 44.416792,-72.221439 44.416564,-72.22084 44.416647,-72.220708 44.416898,-72.221268 44.417736,-72.221378 44.418454,-72.220846 44.418454,-72.220492 44.418406,-72.220095 44.418422,-72.219850 44.418865,-72.220006 44.419292,-72.220514 44.419909,-72.220846 44.420637,-72.220581 44.421159,-72.220824 44.421665,-72.221732 44.42214,-72.222374 44.422757,-72.222971 44.422583,-72.223103 44.422282,-72.223259 44.421981,-72.223923 44.42195,-72.224874 44.421491,-72.22514 44.420827,-72.225557 44.420503,-72.226261 44.420157,-72.226305 44.419869,-72.226807 44.419643,-72.226863 44.419618,-72.227333 44.41927,-72.22776 44.41927,-72.228341 44.419488,-72.22864 44.419675,-72.229199 44.420381,-72.229136 44.421066,-72.228977 44.421181,-72.228945 44.421569,-72.228404 44.421958,-72.228297 44.421996,-72.228021 44.422096,-72.227097 44.422119,-72.226936 44.422302,-72.22649 44.422508,-72.225917 44.422645,-72.224831 44.423217,-72.224068 44.42324,-72.223654 44.423537,-72.223654 44.423717,-72.223654 44.423742,-72.223781 44.42381,-72.223908 44.424587,-72.223924 44.424667,-72.227403 44.426083,-72.228008 44.426329,-72.228662 44.426595,-72.229395 44.426894,-72.230747 44.427231,-72.230971 44.427489,-72.234293 44.428436,-72.236972 44.429199,-72.23719 44.430035,-72.238801 44.430515,-72.24147 44.43131,-72.249962 44.43504,-72.252411 44.436005,-72.255865 44.437305,-72.263358 44.440285,-72.268753 44.442368,-72.277045 44.445563,-72.282573 44.447693,-72.292033 44.451282,-72.299916 44.454331,-72.30082 44.45469,-72.321277 44.462572,-72.325974 44.464436,-72.337127 44.468707,-72.341171 44.470234,-72.348117 44.472931,-72.350859 44.473996,-72.358758 44.477037,-72.371940 44.482113,-72.37455 44.483117,-72.374648 44.483164,-72.374779 44.483225,-72.381115 44.485666,-72.381492 44.485688,-72.392298 44.489866,-72.403342 44.494088,-72.418579 44.499928,-72.418900 44.500046,-72.4255 44.502596,-72.426254 44.502888,-72.434315 44.506098,-72.429847 44.512045,-72.428773 44.513475,-72.421543 44.522929,-72.42088 44.523797,-72.419175 44.524889,-72.416181 44.528756,-72.415673 44.529497,-72.411684 44.534382,-72.403416 44.545482,-72.401901 44.547507,-72.398575 44.553005,-72.396373 44.555888,-72.394723 44.556887,-72.389997 44.563861,-72.38785 44.567083,-72.374786 44.584125,-72.374551 44.584042,-72.374141 44.583887,-72.364296 44.579947,-72.363627 44.579974,-72.362284 44.579484,-72.354192 44.576537,-72.351726 44.575587,-72.346763 44.573679,-72.326173 44.566333,-72.322536 44.56502,-72.312109 44.560996,-72.303198 44.557718,-72.30252 44.557467,-72.299574 44.556334,-72.28218 44.550137,-72.282083 44.550101,-72.280975 44.54965,-72.278823 44.548773,-72.268653 44.544573,-72.268148 44.544564,-72.267242 44.544547,-72.267178 44.544523,-72.266593 44.544303,-72.264975 44.543192,-72.263435 44.542639,-72.263284 44.542541,-72.262964 44.542336,-72.261344 44.544238,-72.259827 44.54559,-72.257865 44.548899,-72.254187 44.554001,-72.249372 44.560675,-72.241644 44.570815,-72.237928 44.576139,-72.237433 44.576818,-72.236396 44.578188,-72.226097 44.591711,-72.224712 44.59373,-72.21978 44.600803,-72.218155 44.602206,-72.210135 44.612594,-72.209324 44.613712,-72.208816 44.614535,-72.207016 44.617438,-72.202901 44.624073,-72.202293 44.625051,-72.199855 44.62841,-72.198157 44.630644,-72.194619 44.635355,-72.193153 44.637354,-72.19052 44.641163,-72.188343 44.644224,-72.184378 44.649871,-72.182172 44.652973,-72.176 44.661598,-72.173651 44.665011,-72.169648 44.67078,-72.165261 44.676857,-72.163971 44.678423,-72.162882 44.679959,-72.158868 44.685588,-72.156736 44.688747,-72.152374 44.694988,-72.150094 44.698355,-72.147612 44.7018,-72.147155 44.702567,-72.146873 44.703032,-72.146226 44.703325,-72.145589 44.702653,-72.141899 44.701438,-72.132779 44.698434,-72.128309 44.696934,-72.124544 44.695638,-72.110654 44.691192,-72.109358 44.690747,-72.088864 44.683714,-72.09305 44.694921,-72.094078 44.697511,-72.094433 44.698327,-72.09461 44.698776,-72.094955 44.699515,-72.095146 44.700011,-72.099952 44.712285,-72.100813 44.714476,-72.101332 44.715715,-72.101783 44.717146,-72.108368 44.733658,-72.114503 44.749624,-72.091369 44.738322,-72.083693 44.734606,-72.061312 44.723479,-72.039137 44.7125,-72.036351 44.711121,-72.02562 44.705812,-72.024818 44.705401,-72.012503 44.698821,-72.010696 44.699614,-72.007273 44.70299,-72.006749 44.703702,-72.005453 44.70441,-72.004999 44.704665,-72.002948 44.706065,-72.002574 44.706344,-71.998947 44.710903,-71.9954 44.715164,-71.995032 44.715247,-71.989775 44.720625,-71.988815 44.721224,-71.981344 44.728822,-71.98093 44.729224,-71.977211 44.732588,-71.976089 44.733763,-71.974770 44.735186,-71.974591 44.73538,-71.973434 44.736503,-71.97039 44.73946,-71.959927 44.749625,-71.958846 44.750086,-71.950606 44.758585,-71.950276 44.758952,-71.943595 44.765337,-71.939936 44.769032,-71.930968 44.765049,-71.928756 44.762738,-71.921857 44.759843,-71.902374 44.750318,-71.882733 44.740608,-71.882573 44.740269,-71.881573 44.739354,-71.876888 44.736935,-71.875148 44.736035,-71.874539 44.735721,-71.858381 44.72766,-71.837294 44.716637,-71.83775 44.716201,-71.852267 44.702366,-71.856071 44.69874,-71.856484 44.698345,-71.87445 44.681027,-71.881231 44.674373,-71.882862 44.67311,-71.883675 44.67175,-71.88436 44.671401,-71.885978 44.669571,-71.888314 44.667829,-71.890624 44.665594,-71.891007 44.664845,-71.898726 44.657917,-71.902653 44.654205,-71.910085 44.64718,-71.910282 44.647038,-71.907401 44.645283,-71.894791 44.638722,-71.89426 44.638626,-71.892962 44.637931,-71.874538 44.628066,-71.86852 44.625053,-71.848306 44.614173,-71.848221 44.614129,-71.842803 44.611203,-71.842732 44.611165,-71.843021 44.610884,-71.856079 44.597255,-71.856286 44.59707,-71.874003 44.580903,-71.874537 44.580409,-71.898111 44.557857,-71.89867 44.557286,-71.908656 44.54708,-71.884835 44.524403,-71.884487 44.524048,-71.874412 44.51323,-71.871838 44.510637,-71.871760 44.51054,-71.861209 44.500056,-71.856636 44.496178,-71.8567 44.496122,-71.874535 44.480282,-71.885627 44.470472,-71.891883 44.464859,-71.893164 44.46383,-71.903342 44.454655,-71.903549 44.454476,-71.912995 44.450245,-71.921348 44.446484,-71.922164 44.44623,-71.925058 44.444914,-71.926307 44.44418,-71.933287 44.441174,-71.91481 44.423081,-71.90842 44.416928,-71.905891 44.414322,-71.898406 44.407105,-71.897962 44.406696,-71.897325 44.406109,-71.886305 44.395364,-71.874506 44.383831,-71.867904 44.377301,-71.865579 44.374962,-71.858643 44.368062,-71.849854 44.359674,-71.847478 44.357361,-71.844776 44.354732,-71.841775 44.351811,-71.840508 44.350578,-71.837656 44.347801,-71.838072 44.347593,-71.83875 44.34721,-71.839767 44.346579,-71.840775 44.345795,-71.84159 44.3453,-71.842483 44.344757,-71.843283 44.34444,-71.844319 44.344204,-71.845288 44.343871,-71.845882 44.343666,-71.846346 44.343506,-71.848005 44.342766,-71.849099 44.342345,-71.849899 44.341978,-71.851118 44.341405,-71.85184 44.341143,-71.852628 44.340873,-71.85355 44.340717,-71.854404 44.340617,-71.855257 44.340589,-71.85619 44.340522,-71.857168 44.340463,-71.858155 44.340363,-71.859166 44.340369,-71.859940 44.340357,-71.860974 44.340282,-71.861941 44.340109,-71.86265 44.33992,-71.863291 44.339778,-71.863719 44.339644,-71.864485 44.339374,-71.865196 44.339071,-71.865895 44.338801,-71.866458 44.338514,-71.866753 44.338322,-71.867509 44.337964,-71.868253 44.337638,-71.869120 44.337288,-71.869616 44.337129,-71.869909 44.336962,-71.870573 44.336804,-71.871203 44.336686,-71.871488 44.336689,-71.871833 44.336641,-71.872472 44.336628,-71.873009 44.336736,-71.873713 44.336973,-71.874326 44.337195,-71.874535 44.337123,-71.874997 44.337226,-71.875514 44.337266,-71.875863 44.33737,-71.876122 44.337482,-71.8762 44.337539,-71.87629 44.337651,-71.876313 44.337732,-71.876324 44.337844,-71.876324 44.337925,-71.876358 44.338054,-71.876403 44.338167,-71.876448 44.338287,-71.876583 44.338424,-71.876685 44.338512,-71.876731 44.338548,-71.876973 44.338572,-71.877493 44.338615,-71.877628 44.338623,-71.877965 44.338719"
        ",-71.878695 44.338964,-71.879896 44.339367,-71.880546 44.339559,-71.880962 44.33976,-71.881154 44.33988,-71.881895 44.340209,-71.883973 44.341228,-71.884917 44.341717,-71.885044 44.341794,-71.887531 44.3426,-71.888182 44.342807,-71.888483 44.342846,-71.888866 44.342916,-71.88893 44.342938,-71.888994 44.342945,-71.889404 44.343039,-71.889893 44.343174,-71.890324 44.343274,-71.890332 44.343272,-71.890380 44.343255,-71.890408 44.343246,-71.890456 44.343228,-71.890548 44.343209,-71.890644 44.343192,-71.890739 44.343177,-71.890844 44.343173,-71.890926 44.343201,-71.891373 44.343335,-71.892197 44.343693,-71.894326 44.344591,-71.895856 44.345209,-71.898726 44.346251,-71.900162 44.346722,-71.902332 44.347499,-71.903115 44.347694,-71.905036 44.348154,-71.906001 44.348239,-71.906909 44.348284,-71.917008 44.346714,-71.917434 44.346535,-71.918748 44.345555,-71.921314 44.343835,-71.921459 44.343739,-71.924607 44.342252,-71.925088 44.342024,-71.926666 44.340286,-71.928041 44.338516,-71.929109 44.337577,-71.932138 44.336541,-71.935395 44.33577,-71.936773 44.335684,-71.939049 44.335843,-71.942442 44.336805,-71.943365 44.337188,-71.944254 44.337643,-71.945162 44.337744,-71.952593 44.337689,-71.956516 44.337632,-71.958119 44.337544,-71.959965 44.337013,-71.961822 44.336634,-71.963133 44.336556,-71.972572 44.336781,-71.973300 44.336777,-71.977175 44.337518,-71.977971 44.33757,-71.979505 44.337354,-71.980559 44.337486,-71.98112 44.3375,-71.984281 44.336414,-71.984617 44.336243,-71.984729 44.335882,-71.986483 44.331218,-71.987862 44.330081,-71.988305 44.329768,-71.989058 44.329561,-71.992446 44.328548,-71.993662 44.327611,-71.999448 44.325595,-72.000793 44.325295,-72.001221 44.325234,-72.001792 44.325079,-72.002314 44.324871,-72.003547 44.324397,-72.00485 44.324054,-72.005532 44.323829,-72.00676 44.323357,-72.009976 44.321951,-72.012172 44.321408,-72.014543 44.321032,-72.015936 44.320849,-72.01913 44.320383,-72.019577 44.320498,-72.019801 44.32052,-72.019908 44.320683,-72.020149 44.320818,-72.021041 44.320887,-72.021994 44.321184,-72.025782 44.322054,-72.028201 44.322375,-72.029061 44.322398,-72.029698 44.32217,-72.031322 44.320936,-72.031959 44.320662,-72.033136 44.320365,-72.033137 44.319543,-72.033773 44.317989,-72.033806 44.317349,-72.033678 44.316823,-72.03317 44.31632,-72.032341 44.315752,-72.031937 44.315475,-72.031802 44.315383,-72.031739 44.314652,-72.031549 44.313966,-72.031708 44.313007,-72.032218 44.311955,-72.032505 44.310653,-72.032444 44.307134,-72.032316 44.306677,-72.032541 44.303752,-72.032955 44.302701,-72.033464 44.301878,-72.034324 44.300941,-72.03487 44.300512,-72.035660 44.29989,-72.03633 44.298634,-72.03703 44.297834,-72.039003 44.296463,-72.039836 44.296087,-72.040117 44.295962,-72.040543 44.295876,-72.041609 44.295665,-72.041856 44.29565,-72.043202 44.295579,-72.043775 44.295608,-72.044546 44.294969,-72.044926 44.294636,-72.045137 44.29391,-72.045265 44.293198,-72.045745 44.292448,-72.046302 44.291983,-72.047349 44.291581,-72.048216 44.291404,-72.049472 44.291186,-72.051541 44.290889,-72.053355 44.290501,-72.054046 44.289971,-72.055002 44.288945,-72.055964 44.288162,-72.056078 44.287893,-72.056409 44.287618,-72.057273 44.287163,-72.05888 44.28624,-72.059957 44.284767,-72.062298 44.281754,-72.065434 44.277235,-72.066464 44.275093,-72.067774 44.270976,-72.066464 44.268331,-72.065949 44.268666,-72.064544 44.267997,-72.062671 44.269336,-72.060846 44.269972,-72.05874 44.270005,-72.058646 44.269269,-72.059395 44.268365,-72.05888 44.266926,-72.059113 44.265787,-72.058553 44.265285,-72.059021 44.265018,-72.059581 44.264315,-72.059535 44.265687,-72.060378 44.264951,-72.061173 44.263377,-72.061033 44.26244,-72.060333 44.261643,-72.060398 44.261463,-72.060723 44.261313,-72.061173 44.261258,-72.061495 44.260726,-72.06156 44.260621,-72.061542 44.26047,-72.060878 44.260436,-72.060518 44.260284,-72.060307 44.259961,-72.060259 44.259789,-72.060163 44.259766,-72.060194 44.257778,-72.060099 44.257595,-72.059838 44.256295,-72.059782 44.256018,-72.059523 44.255798,-72.059432 44.255721,-72.059018 44.255013,-72.058161 44.253916,-72.058224 44.253823,-72.058033 44.253664,-72.05708 44.252064,-72.056889 44.251859,-72.055694 44.250061,-72.055641 44.249649,-72.055515 44.249615,-72.055483 44.24912,-72.055134 44.248435,-72.054276 44.247703,-72.05399 44.246926,-72.051002 44.244892,-72.050112 44.244046,-72.049604 44.243292,-72.048999 44.242058,-72.04846 44.241212,-72.04792 44.239498,-72.047889 44.238493,-72.048621 44.237145,-72.050209 44.235066,-72.050656 44.233581,-72.052118 44.229856,-72.052469 44.229239,-72.052532 44.228805,-72.053264 44.227114,-72.053581 44.22604,-72.053995 44.224028,-72.0539 44.222703,-72.053614 44.221081,-72.053518 44.221035,-72.053518 44.220852,-72.053423 44.220784,-72.053423 44.220601,-72.053328 44.220487,-72.053328 44.220327,-72.053043 44.219732,-72.052662 44.218841,-72.052966 44.217794,-72.053232 44.216876,-72.05495 44.213677,-72.055903 44.212694,-72.056952 44.212169,-72.058605 44.211803,-72.058954 44.211597,-72.059716 44.210752,-72.060067 44.209998,-72.060067 44.209816,-72.058987 44.208512,-72.058605 44.208215,-72.058542 44.208032,-72.05886 44.207667,-72.058987 44.207164,-72.058906 44.20701,-72.058796 44.206798,-72.058066 44.206067,-72.058194 44.204907,-72.058287 44.204056,-72.058987 44.202114,-72.059564 44.201223,-72.059614 44.201144,-72.059669 44.201061,-72.060067 44.200446,-72.060543 44.200012,-72.062035 44.199074,-72.063560 44.198457,-72.064005 44.198069,-72.064577 44.196949,-72.064608 44.196058,-72.064608 44.195692,-72.06499 44.194801,-72.065213 44.194115,-72.065403 44.192607,-72.066102 44.19039,-72.066166 44.189773,-72.065911 44.189293,-72.065085 44.188631,-72.064285 44.187888,-72.06299 44.186688,-72.061337 44.184951,-72.05956 44.182666,-72.059116 44.181912,-72.058956 44.18182,-72.057496 44.179444,-72.056987 44.178096,-72.056861 44.178027,-72.056862 44.177822,-72.056798 44.17773,-72.056798 44.177433,-72.056575 44.176679,-72.056480 44.176633,-72.05629 44.175742,-72.054734 44.173114,-72.054227 44.172108,-72.054005 44.170646,-72.05391 44.170577,-72.053783 44.169869,-72.053021 44.167903,-72.052196 44.166921,-72.051655 44.166463,-72.050575 44.165961,-72.049179 44.165138,-72.048925 44.164795,-72.048799 44.164498,-72.048672 44.164452,-72.048639 44.164201,-72.048418 44.163926,-72.048355 44.163058,-72.048101 44.162144,-72.047592 44.161801,-72.047022 44.161595,-72.046133 44.161549,-72.043244 44.161869,-72.042830 44.161754,-72.042386 44.161343,-72.042386 44.160817,-72.042672 44.160292,-72.043785 44.158852,-72.044102 44.158121,-72.044039 44.157435,-72.043785 44.157047,-72.0435 44.156841,-72.042801 44.156613,-72.042103 44.15659,-72.041119 44.156864,-72.040167 44.157023,-72.039594 44.156749,-72.039245 44.156703,-72.039150 44.156498,-72.03988 44.155932,-72.039976 44.155858,-72.040082 44.155749,-72.040737 44.155972,-72.041923 44.156312,-72.042043 44.156349,-72.042801 44.156613,-72.044676 44.157097,-72.044884 44.157151,-72.045078 44.157206,-72.045767 44.157401,-72.046234 44.157563,-72.047344 44.157836,-72.048554 44.158134,-72.058602 44.160893,-72.065528 44.162794,-72.066594 44.163099,-72.070984 44.164364,-72.083157 44.167671,-72.083344 44.167722,-72.083451 44.167775,-72.087279 44.168733,-72.091724 44.16998,-72.092625 44.170284,-72.097954 44.171765,-72.100237 44.172391,-72.100809 44.172548,-72.100996 44.172599,-72.101265 44.172673,-72.10189 44.172759,-72.102047 44.172803,-72.105729 44.173842,-72.109921 44.175046,-72.110927 44.175333,-72.116790 44.176949,-72.122705 44.178552,-72.124310 44.179174,-72.133395 44.181725,-72.138984 44.183365,-72.139194 44.183494,-72.139466 44.183503,-72.140796 44.183868,-72.151283 44.186773,-72.166147 44.190938,-72.16836 44.191508,-72.17026 44.192053,-72.171978 44.19207,-72.174381 44.192183,-72.175395 44.192186,-72.187153 44.191379,-72.190945 44.191111,-72.191322 44.191101,-72.192686 44.190957,-72.199012 44.190573,-72.204065 44.190303,-72.209594 44.190003,-72.210965 44.189741,-72.221076 44.189175,-72.229408 44.188625,-72.237321 44.188057,-72.238117 44.188023,-72.24954 44.187528,-72.271323 44.185807,-72.282972 44.184887,-72.297814 44.183545))";
    test_self_overlay<polygon>("uscounty_ne", 1, uscounty_ne, false);

    // First two points are similar but not equal, causing a self-intersection (continuation)
    //   ----s-C------
    // C=closing point, s=similar point, lying a little south-west. This causes a c/c (continue) intersection point.
    // Was caused by error in neighbouring (get_turns.cpp)
    std::string const ticket_9081_20873 = 
        "POLYGON((0.5246796698528305436  0.56288112949742163948,"
                 "0.52467966985283021053 0.56288112949742141744,"
                                                                   "0.51150490059304598578 0.55384245388529118603,0.51053725440836283944 0.56288079068493779289,"
                                                                   "0.55019180962335967333 0.60735187918348676472,0.5445123657939807682 0.58331435417450905323,"
                 "0.5246796698528305436  0.56288112949742163948))";
    test_self_overlay<polygon>("ticket_9081_20873", 0, ticket_9081_20873);

    // Was caused by error in neighbouring (get_turns.cpp)
    std::string const ggl_list_2013_11_06_joan = "POLYGON((137.14632454923444 200.61927877947369,50 224, 323 497,255 169,137.14632454923444 200.61927877947369))";
    test_self_overlay<polygon>("ggl_list_2013_11_06_joan", 0, ggl_list_2013_11_06_joan);

    // Same case - but if this is a linestring. 
    // TODO: this does not compile yet, but it should return 1 intersection point at the "closing" point
    // std::string const ggl_list_2013_11_06_joan_linestring = "LINESTRING(137.14632454923444 200.61927877947369,50 224, 323 497,255 169,137.14632454923444 200.61927877947369)";
    // test_self_overlay<linestring>("ggl_list_2013_11_06_joan_linestring", 1, ggl_list_2013_11_06_joan_linestring);
}


int test_main(int, char* [])
{
    test_self_all<bg::model::d2::point_xy<double> >();
    return 0;
}
