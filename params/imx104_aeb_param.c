#include "img_struct.h"

statistics_config_t imx104_tile_config = {
	1,
	1,

	24,
	16,
	128,
	48,
	160,
	250,
	160,
	250,
	0,
	16383,

	12,
	8,
	0,
	8,
	340,
	512,

	8,
	5,
	256,
	10,
	448,
	816,
	448,
	816,

	0,
	16383,
};

line_t imx104_50hz_lines[] = {
		{
		{{SHUTTER_1BY8000, ISO_100, 0}}, {{SHUTTER_1BY100, ISO_100,0}}
		},

		{
		{{SHUTTER_1BY100, ISO_100, 0}}, {{SHUTTER_1BY100, ISO_300, 0}}
		},

		{
		{{SHUTTER_1BY50, ISO_100, 0}}, {{SHUTTER_1BY50, ISO_800,0}}
		},

		{
		{{SHUTTER_1BY30, ISO_100, 0}}, {{SHUTTER_1BY30, ISO_102400,0}}
		}
};

line_t imx104_60hz_lines[] = {
		{
		{{SHUTTER_1BY8000, ISO_100, 0}}, {{SHUTTER_1BY120, ISO_100,0}}
		},

		{
		{{SHUTTER_1BY120, ISO_100, 0}}, {{SHUTTER_1BY120, ISO_300, 0}}
		},

		{
		{{SHUTTER_1BY60, ISO_100, 0}}, {{SHUTTER_1BY60, ISO_800,0}}
		},

		{
		{{SHUTTER_1BY30, ISO_100, 0}}, {{SHUTTER_1BY30, ISO_102400,0}}
		}
};
 
img_awb_param_t imx104_awb_param = {
	{
		{2150, 1024, 1650},	//AUTOMATIC
		{1250, 1024, 3200},	//INCANDESCENT
		{1600, 1024, 2600},	//D4000
		{1850, 1024, 2000},	//D5000
		{2150, 1024, 1750},	//SUNNY
		{2300, 1024, 1650},	//CLOUDY
		{1598, 1024, 1875},	//FLASH
		{1024, 1024, 1024},	//FLUORESCENT
		{1024, 1024, 1024},	//FLUORESCENT_H
		{1024, 1024, 1024},	//UNDER WATER
		{1024, 1024, 1024},	//CUSTOM
		{1598, 1024, 1875},	//AUTOMATIC OUTDOOR
	},
	{
		12,
		{{ 900,1600,2700,3700, -2500, 5300, -2500,7200, 1000,1500, 1000,2600, 1},	// 0	INCANDESCENT
		 { 1100,2100,2000,3200,-2200,5000, -2500,8200, 1000,500, 1000,1600, 2},	// 1    D4000
		 {1500,2200,1550,2450,-1700,4400, -2000,6400, 1000,-200, 1000,600, 4},	// 2	 D5000
		 {1800,2500, 1400,2100,   -800,3000,   -800,3750, 1000,-500, 1000,-150, 8  },	// 3    SUNNY
		 {2000,2700,1300,2000,   -800,3000,   -800, 3750, 1000,-1100, 1000,-200, 4},	// 4    CLOUDY
		 {   0,    0,    0,    0,     0,    0,     0,    0,   0,   0,    0,    0, 0 },					// 5    ...
		 { 2100,2500,1700,3500,-1400,4800,-1200,5800,1600,-1800,3000,-3300,-1 },		// 6	GREEN REGION
		 {   0,    0,    0,    0,     0,    0,     0,    0,   0,   0,    0,    0, 0 },	// 7    FLASH
		 {   0,    0,    0,    0,     0,    0,     0,    0,   0,   0,    0,    0, 0 },	// 8	FLUORESCENT
		 {   0,    0,    0,    0,     0,    0,     0,    0,   0,   0,    0,    0, 0 },	// 9    FLUORESCENT_2
		 {   0,    0,    0,    0,     0,    0,     0,    0,   0,   0,    0,    0, 0 },	// 10	FLUORESCENT_3
		 {   0,    0,    0,    0,     0,    0,     0,    0,   0,   0,    0,    0, 0 }},
	},
	{	{ 0 ,6},	//LUT num. AUTOMATIC  INDOOR
		{ 0, 1},	//LUT num. INCANDESCENT
		{ 1, 1},	//LUT num. D4000
		{ 2, 1},	//LUT num. D5000
		{ 2, 5},	//LUT num. SUNNY
		{ 4, 3},	//LUT num. CLOUDY
		{ 7, 1},	//LUT num. FLASH
		{ 8, 1},	//LUT num. FLUORESCENT
		{ 9, 1},	//LUT num. FLUORESCENT_H
		{11, 1},	//LUT num. UNDER WATER
		{11, 1},	//LUT num. CUSTOM
		{ 0, 7},	//LUT num. AUTOMATIC  OUTDOOR
	 }
};

// copy from imx036
u32 imx104_ae_agc_dgain[AGC_DGAIN_TABLE_LENGTH]={
		1024,
		1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
		1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
		1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
		1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
		1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
		1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
		1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
		1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
		1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
		1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
		1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
		1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
		1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
		1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
		1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
		1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
		1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
		1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
		1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
		1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
		1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
		1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
		1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
		1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
		1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
		1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
		1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
		1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
		1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
		1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
		1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
		1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
		1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
		1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
		1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
		1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
		1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
		1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
		1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
		1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
		1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
		1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
		1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
		1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
		1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
		1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
		1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
		1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	};

u32 imx104_ae_sht_dgain_29_97[SHUTTER_DGAIN_TABLE_LENGTH]={
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,
	1024,1018,1029,1038,1019,1020,1046,1026,1035,1024,1039,1044,1020,1024,1045,1030,
	1041,1025,1024,1052,1028,1034,1024,1038,1046,1025,1026,1027,1033,1042,1030,1021,
	1029,1033,1036,1035,1016,1024,1039,1033,1041,1020,1024,1053,1033,1042,1031,1024,
	1062,1040,1043,1045,1024,1061,1050,1038,1045,1024,1055,1059,1037,1044,1038,1024,
	1063,1044,1040,1044,1019,1024,1054,1037,1046,1031,1025,1032,1044,1047,1047,1026,
	1034,1024,1050,1060,1040,1040,1046,1024,1068,1064,1046,1055,1041,1033,1041,1059,
	1059,1063,1040,1045,1041,1024,1073,1059,1049,1057,1035,1035,1038,1056,1062,1057,
	1039,1046,1032,1024,1076,1054,1056,1059,1037,1044,1037,1024,1079,1061,1057,1063,
	1040,1044,1043,1024,1080,1067,1058,1066,1044,1044,1047,1024,1081,1074,1057,1065,
	1059,1066,1046,1043,1048,1076,1080,1077,1056,1063,1050,1040,1047,1024,1080,1080,
	1056,1062,1052,1038,1045,1023,1022,1024,1060,1067,1059,1046,1055,1033,1035,1036,
	1016,1024,1073,1066,1073,1049,1056,1048,1035,1044,1022,1024,1024,1072,1082,1064,
	1062,1067,1044,1052,1041,1034,1041,1018,1024,1091,1076,1087,1066,1067,1069,1049,
	1058,1044,1040,1046,1024,1107,1100,1087,1096,1073,1075,1075,1055,1064,1047,1043,
	1048,1024,1114,1104,1092,1099,1074,1076,1073,1053,1062,1043,1038,1041,1017,1024,
	1104,1094,1102,1078,1082,1080,1061,1070,1052,1049,1052,1029,1036,1026,1016,1024,
	1100,1105,1104,1086,1097,1080,1078,1083,1061,1070,1060,1051,1060,1036,1042,1038,
	1024,1145,1123,1124,1125,1103,1113,1096,1091,1096,1072,1079,1069,1057,1066,1043,
	1045,1043,1024,1160,1141,1138,1143,1118,1126,1117,1104,1113,1089,1092,1091,1070,
	1079,1064,1058,1063,1039,1045,1038,1024,1177,1154,1155,1157,1133,1142,1128,1121,
	1128,1102,1108,1103,1087,1096,1076,1074,1077,1054,1061,1050,1040,1047,1024,1195,
	1190,1171,1181,1159,1156,1161,1135,1143,1136,1143,1117,1119,1119,1096,1104,1091,
	1080,1087,1063,1064,1064,1041,1047,1037,1024,1232,1208,1207,1211,1184,1192,1184,
	1167,1177,1156,1152,1157,1131,1136,1131,1112,1120,1103,1096,1103,1077,1081,1078,
	1058,1065,1050,1042,1048,1024,1026,1024,1246,1256,1240,1231,1240,1212,1217,1216,
	1193,1203,1187,1179,1186,1160,1164,1161,1141,1151,1133,1127,1133,1107,1113,1106,
	1089,1098,1078,1075,1079,1055,1061,1051,1038,1046,1024,1024,1024,1319,1331,1317,
	1305,1315,1287,1291,1291,1265,1277,1260,1252,1261,1233,1238,1236,1213,1223,1205,
	1198,1205,1178,1183,1179,1159,1169,1150,1145,1151,1124,1130,1124,1107,1116,1096,
	1092,1096,1071,1077,1069,1054,1062,1042,1039,1042,1018,1024,1485,1467,1480,1451,
	1451,1456,1425,1434,1423,1407,1419,1391,1392,1396,1365,1375,1363,1348,1358,1331,
	1331,1334,1305,1314,1301,1288,1297,1269,1271,1272,1245,1254,1239,1229,1236,1209,
	1212,1211,1187,1197,1179,1170,1176,1148,1153,1148,1128,1137,1117,1111,1115,1089,
	1094,1086,1069,1076,1055,1051,1053,1028,1033,1024,1874,1890,1855,1854,1863,1822,
	1836,1823,1801,1817,1784,1782,1789,1751,1762,1750,1727,1742,1712,1708,1715,1676,
	1687,1678,1652,1667,1638,1633,1640,1603,1612,1602,1578,1591,1564,1559,1564,1529,
	1538,1526,1506,1517,1487,1486,1488,1455,1465,1449,1436,1444,1411,1415,1410,1385,
	1395,1371,1364,1369,1336,1343,1330,1314,1322,1294,1294,1291,1264,1273,1255,1243,
	1248,1219,1221,1215,1192,1200,1178,1171,1175,1145,1149,1139,1119,1126,1102,1098,
	1099,1071,1076,1063,1048,1052,1027,1024,6524,6066,5763,5371,4983,4669,4262,3920,
	3575,3179,2846,2469,2096,1757,1374,1024,

};

u32 imx104_ae_sht_dgain_25[SHUTTER_DGAIN_TABLE_LENGTH]={
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,
	1024,1019,1037,1033,1028,1024,1041,1035,1030,1024,1043,1038,1033,1028,1024,1042,
	1036,1030,1024,1044,1039,1034,1029,1024,1043,1036,1030,1024,1045,1040,1034,1029,
	1024,1045,1040,1034,1029,1024,1046,1041,1035,1030,1024,1046,1041,1035,1029,1024,
	1047,1041,1035,1029,1024,1049,1044,1039,1034,1029,1024,1049,1043,1036,1030,1024,
	1051,1045,1040,1035,1029,1024,1052,1046,1041,1035,1029,1024,1053,1047,1041,1035,
	1029,1024,1054,1048,1042,1036,1030,1024,1056,1051,1045,1040,1035,1029,1024,1057,
	1052,1046,1041,1035,1029,1024,1058,1053,1046,1041,1035,1029,1024,1060,1053,1047,
	1040,1034,1029,1024,1062,1056,1050,1045,1039,1034,1029,1024,1064,1058,1053,1046,
	1041,1035,1030,1024,1067,1061,1055,1050,1045,1039,1035,1029,1024,1069,1064,1059,
	1052,1046,1038,1031,1024,1071,1066,1060,1055,1049,1044,1039,1034,1028,1024,1073,
	1066,1060,1054,1047,1041,1035,1029,1024,1078,1072,1067,1061,1056,1050,1045,1040,
	1035,1029,1024,1081,1074,1068,1062,1056,1049,1043,1036,1030,1024,1086,1081,1076,
	1071,1065,1060,1054,1048,1042,1036,1030,1024,1090,1083,1077,1071,1065,1058,1052,
	1046,1041,1035,1029,1024,1095,1088,1082,1076,1070,1064,1058,1052,1047,1041,1035,
	1029,1024,1102,1097,1091,1086,1080,1074,1069,1064,1058,1052,1047,1041,1035,1030,
	1024,1110,1105,1099,1093,1087,1081,1076,1069,1063,1057,1050,1043,1037,1030,1024,
	1118,1111,1105,1099,1092,1086,1079,1073,1067,1061,1055,1050,1044,1039,1034,1028,
	1024,1129,1123,1117,1111,1104,1098,1092,1086,1080,1073,1067,1061,1054,1048,1042,
	1036,1030,1024,1145,1140,1135,1131,1126,1121,1116,1110,1105,1098,1092,1086,1080,
	1073,1066,1059,1051,1044,1037,1030,1024,1158,1150,1143,1136,1129,1122,1115,1109,
	1103,1098,1092,1086,1081,1076,1069,1064,1058,1052,1047,1041,1035,1029,1024,1184,
	1178,1171,1166,1160,1154,1149,1143,1137,1130,1123,1117,1110,1103,1096,1090,1083,
	1076,1070,1063,1057,1050,1044,1037,1030,1024,1215,1208,1202,1196,1190,1183,1177,
	1170,1164,1157,1151,1145,1138,1132,1126,1120,1113,1107,1100,1094,1087,1081,1075,
	1068,1061,1055,1050,1043,1037,1030,1024,1263,1255,1249,1242,1236,1229,1222,1215,
	1209,1202,1196,1188,1182,1176,1169,1162,1156,1149,1143,1136,1129,1123,1116,1110,
	1103,1097,1090,1083,1076,1070,1063,1057,1050,1043,1036,1030,1024,1340,1333,1326,
	1320,1314,1307,1301,1295,1288,1280,1274,1268,1261,1255,1247,1240,1233,1226,1219,
	1213,1206,1199,1191,1184,1178,1170,1164,1157,1150,1143,1136,1129,1122,1114,1108,
	1101,1093,1087,1080,1072,1065,1059,1051,1044,1037,1030,1024,1487,1479,1471,1464,
	1456,1449,1441,1433,1425,1416,1410,1402,1394,1387,1379,1372,1364,1356,1348,1340,
	1332,1324,1317,1310,1302,1294,1286,1279,1272,1264,1256,1248,1241,1233,1226,1218,
	1210,1203,1196,1188,1180,1172,1165,1157,1150,1142,1135,1128,1120,1112,1105,1098,
	1091,1083,1075,1068,1061,1053,1046,1038,1031,1024,1887,1878,1869,1860,1849,1841,
	1831,1823,1812,1802,1792,1784,1775,1764,1754,1747,1737,1727,1717,1709,1699,1691,
	1682,1672,1662,1655,1642,1635,1625,1615,1607,1597,1588,1580,1570,1560,1549,1542,
	1532,1522,1512,1504,1494,1485,1474,1466,1454,1446,1437,1427,1419,1410,1399,1390,
	1380,1372,1363,1353,1342,1334,1327,1315,1306,1298,1289,1279,1269,1260,1251,1242,
	1234,1224,1215,1206,1196,1187,1177,1170,1161,1151,1142,1133,1123,1113,1104,1096,
	1088,1078,1070,1060,1051,1042,1032,1024,6452,6121,5757,5365,5016,4652,4300,3936,
	3563,3208,2844,2480,2110,1752,1395,1024,
};

u32 imx104_ae_sht_dgain[SHUTTER_DGAIN_TABLE_LENGTH]={

	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
	1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,
};
u8 imx104_dlight[2] = {128,4};
