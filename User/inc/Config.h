#ifndef _CONFIG_H
#define _CONFIG_H

#define NO2

#define xUSE_BMP

#define xUSE_NEW_FORMAT

#define USE_MODE_SWITCH

#define USE_INC_PID

#define xUSE_ONLY_FTM_QD

#define DOUBLE_CAR

#define DYNAMIC_PRESIGHT

#define xRELIABLE_CONNECTION

#define xPERIODICALLY_CHECK_MSG_QUEUE

#define xDYNAMIC_INIT_LEADER_CAR

#ifdef DOUBLE_CAR
#define xFINAL_OVERTAKING
#else
#define USE_IMG_TRANS
#define xUSE_STAT_TRANS
#endif

#ifdef DYNAMIC_PRESIGHT
#define PRESIGHT_ONLY_DEPENDS_ON_PURSUEING
#endif

#ifndef USE_NEW_FORMAT
#define HIGHLIGHT_PRESIGHT
#define HIGHLIGHT_MIDDLE_LINE
#define HIGHLIGHT_BORDER_LINE
#endif

#endif
