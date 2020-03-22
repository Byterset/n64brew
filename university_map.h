
#ifndef UNIVERSITY_MAP_H
#define UNIVERSITY_MAP_H 1
#include "constants.h"
#include "gameobject.h"


AABB university_map_bounds[] = {
{{-45.071983, -12.775320, -47.049881}, {48.456688, 53.892517, 47.044338}},
{{-45.071983, -12.775320, -47.049881}, {48.456745, 53.892513, 47.044338}},
{{-45.071983, -12.775320, -47.049881}, {48.456688, 53.892517, 47.044338}},
{{-16.400528, 0.000000, -44.529362}, {20.007133, 159.193802, 44.529362}},
{{-236.518585, -54.389256, -15.605621}, {236.519394, 18.310425, 15.605621}},
{{-236.518585, -54.389256, -15.605621}, {236.519394, 18.310425, 15.605621}},
{{-75.067978, -32.971409, -74.530678}, {75.116386, 31.478992, 74.758820}},
{{-75.067978, -32.971409, -74.530678}, {75.116386, 31.478992, 74.758820}},
{{-236.518585, -54.389256, -15.605621}, {236.519394, 18.310425, 15.605621}},
{{-236.518585, -54.389256, -15.605621}, {236.519394, 18.310425, 15.605621}},
{{-45.071869, -12.775320, -47.049866}, {48.456688, 53.892517, 47.044342}},
{{-75.067978, -32.971409, -74.530678}, {75.116386, 31.478992, 74.758820}},
{{-236.518585, -54.389256, -15.605621}, {236.519394, 18.310425, 15.605621}},
{{-32.452812, 0.000000, -15.666361}, {28.539619, 69.391663, 15.363922}},
{{-6.503963, -2.904237, -12.591991}, {10.518951, 2.923790, 12.591991}},
{{0.000000, -12.573848, -1240.013794}, {1240.017700, 0.000000, 137.799683}},
{{0.000000, 0.000000, 0.000000}, {1240.017700, 0.000000, 1240.050659}},
{{-1240.046875, 0.000000, 0.000000}, {137.766846, 5.829808, 1240.050659}},
{{-146.152374, -21.263721, -431.325989}, {688.892273, 0.000000, 137.799683}},
{{-1242.745117, -36.063183, -441.609863}, {0.000000, 22.896681, 137.799683}},
{{-154.187164, -167.989456, -1246.629150}, {689.747437, 0.000000, 0.000000}},
{{-1249.205078, -167.989456, -1249.477417}, {0.000000, 16.296743, 0.000000}},
{{-778.810608, 0.000000, -372.235565}, {778.810730, 0.000000, 372.235565}},

};

GameObject university_map_data[] = {
{0, // object id
{-1060.743896, 11.748462, -52.583786}, // position
{0.000000, 0.000000, -0.000000}, // rotation
BushModel, // modelType
0, // subtype
},
{1, // object id
{-994.538513, 11.748466, -18.517456}, // position
{0.000000, 0.000000, -0.000000}, // rotation
BushModel, // modelType
0, // subtype
},
{2, // object id
{-1028.484131, 11.748462, -134.772263}, // position
{0.000000, 0.000000, -0.000000}, // rotation
BushModel, // modelType
0, // subtype
},
{3, // object id
{-1042.592285, 0.000000, -553.012817}, // position
{0.000000, 0.000000, -0.000000}, // rotation
GardenerCharacterModel, // modelType
0, // subtype
},
{4, // object id
{-940.044006, 49.424400, -691.284058}, // position
{0.000000, 0.000000, -0.000000}, // rotation
WallModel, // modelType
0, // subtype
},
{5, // object id
{-1533.462036, 49.424400, -691.284058}, // position
{0.000000, 0.000000, -0.000000}, // rotation
WallModel, // modelType
0, // subtype
},
{6, // object id
{-1390.277832, 28.706844, -404.714294}, // position
{0.000000, 0.000000, -0.000000}, // rotation
PlanterModel, // modelType
0, // subtype
},
{7, // object id
{-1390.277832, 28.706844, -127.979004}, // position
{0.000000, 0.000000, -0.000000}, // rotation
PlanterModel, // modelType
0, // subtype
},
{8, // object id
{-1239.636597, 49.424400, -879.493408}, // position
{0.000000, 0.000000, -0.000000}, // rotation
WallModel, // modelType
0, // subtype
},
{9, // object id
{-1712.740479, 49.424400, -879.493408}, // position
{0.000000, 0.000000, -0.000000}, // rotation
WallModel, // modelType
0, // subtype
},
{10, // object id
{-1931.657593, 11.748433, -464.240753}, // position
{0.000000, 0.000000, -0.000000}, // rotation
BushModel, // modelType
0, // subtype
},
{11, // object id
{-434.772614, 28.706844, -639.281494}, // position
{0.000000, 0.000000, -0.000000}, // rotation
PlanterModel, // modelType
0, // subtype
},
{12, // object id
{-766.229980, 49.424400, -879.493408}, // position
{0.000000, 0.000000, -0.000000}, // rotation
WallModel, // modelType
0, // subtype
},
{13, // object id
{-1211.296143, -0.405316, -456.490509}, // position
{0.000000, 0.000000, -0.000000}, // rotation
GooseModel, // modelType
0, // subtype
},
{14, // object id
{-900.784485, 0.000007, -241.499634}, // position
{0.000000, 0.000000, -0.000000}, // rotation
BookItemModel, // modelType
0, // subtype
},
{15, // object id
{-1159.037354, -0.059913, -1051.291382}, // position
{0.000000, 0.000000, 0.000000}, // rotation
GroundModel, // modelType
1, // subtype
},
{16, // object id
{-1159.037354, -0.059913, -1051.291382}, // position
{0.000000, 0.000000, 0.000000}, // rotation
GroundModel, // modelType
2, // subtype
},
{17, // object id
{-1159.037354, -0.059913, -1051.291382}, // position
{0.000000, 0.000000, 0.000000}, // rotation
GroundModel, // modelType
3, // subtype
},
{18, // object id
{-1159.037354, -0.059913, -1051.291382}, // position
{0.000000, 0.000000, 0.000000}, // rotation
GroundModel, // modelType
4, // subtype
},
{19, // object id
{-1159.037354, -0.059913, -1051.291382}, // position
{0.000000, 0.000000, 0.000000}, // rotation
GroundModel, // modelType
5, // subtype
},
{20, // object id
{-1159.037354, -0.059913, -1051.291382}, // position
{0.000000, 0.000000, 0.000000}, // rotation
GroundModel, // modelType
6, // subtype
},
{21, // object id
{-1159.037354, -0.059913, -1051.291382}, // position
{0.000000, 0.000000, 0.000000}, // rotation
GroundModel, // modelType
7, // subtype
},
{22, // object id
{-1623.523560, -65.354485, -1919.260742}, // position
{0.000000, 0.000000, 0.000000}, // rotation
WaterModel, // modelType
0, // subtype
},

};

#define UNIVERSITY_MAP_COUNT 23

#endif /* UNIVERSITY_MAP_H */
