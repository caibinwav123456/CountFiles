#ifndef _DEFINES_H_
#define _DEFINES_H_

#define DEF_LINE_HEIGHT        20
#define DEF_LINE_INDENT        20
#define DEF_TEXT_HEIGHT        16
#define DEF_FOLDER_METRIC      24
#define DEF_CONN_START         10
#define DEF_CONN_END           20
#define DEF_CONN_Y             10
#define DEF_FILE_WIDTH         12
#define DEF_MIN_SCROLL_WIDTH   294
#define DEF_BAR_CENTER_MARGIN  5
#define DEF_BAR_CENTER_SPACE   38

#define DEF_DEFAULT_TABWIDTH_NAME  200
#define DEF_DEFAULT_TABWIDTH_SIZE  110
#define DEF_DEFAULT_TABWIDTH_TIME  160
#define DEF_MIN_TABWIDTH_NAME      80
#define DEF_MIN_TABWIDTH_OTHER     10
#define DEF_TAB_GRAB_BIAS          5

#define DEF_MODIFY_DATE_PART_WIDTH 105
#define DEF_TABMARGIN_SIZE     10

#define DEF_FILE_RECT          CRect(3,6,9,14)

#define DEF_PROP_WND_HEIGHT    32
#define DEF_PROP_START_X       4
#define DEF_PAGE_TAB_RECT      CRect(0,2,275,31)
#define DEF_PAGE_STRING_RECT   CRect(29,2,227,31)
#define DEF_PAGE_TAB_WIDTH     250
#define DEF_MIN_PROP_LEFT_MARGIN 100
#define DEF_MIN_PROP_RIGHT_MARGIN 70
#define DEF_PROP_BUTTON_RECT   CRect(232,7,245,20)

#define PAGE_TAB_MARGIN        35

#ifndef USE_HIPPI_SCREEN

#define LINE_HEIGHT        DEF_LINE_HEIGHT
#define LINE_INDENT        DEF_LINE_INDENT
#define TEXT_HEIGHT        DEF_TEXT_HEIGHT
#define FOLDER_METRIC      DEF_FOLDER_METRIC
#define CONN_START         DEF_CONN_START
#define CONN_END           DEF_CONN_END
#define CONN_Y             DEF_CONN_Y
#define FILE_WIDTH         DEF_FILE_WIDTH
#define MIN_SCROLL_WIDTH   DEF_MIN_SCROLL_WIDTH
#define BAR_CENTER_MARGIN  DEF_BAR_CENTER_MARGIN
#define BAR_CENTER_SPACE   DEF_BAR_CENTER_SPACE

#define DEFAULT_TABWIDTH_NAME  DEF_DEFAULT_TABWIDTH_NAME
#define DEFAULT_TABWIDTH_SIZE  DEF_DEFAULT_TABWIDTH_SIZE
#define DEFAULT_TABWIDTH_TIME  DEF_DEFAULT_TABWIDTH_TIME
#define MIN_TABWIDTH_NAME      DEF_MIN_TABWIDTH_NAME
#define MIN_TABWIDTH_OTHER     DEF_MIN_TABWIDTH_OTHER
#define TAB_GRAB_BIAS          DEF_TAB_GRAB_BIAS

#define MODIFY_DATE_PART_WIDTH DEF_MODIFY_DATE_PART_WIDTH
#define TABMARGIN_SIZE         DEF_TABMARGIN_SIZE

#define FILE_RECT          DEF_FILE_RECT

#define PROP_WND_HEIGHT    DEF_PROP_WND_HEIGHT
#define PROP_START_X       DEF_PROP_START_X
#define PAGE_TAB_RECT      DEF_PAGE_TAB_RECT
#define PAGE_TAB_MARGIN_SCALED PAGE_TAB_MARGIN
#define PAGE_STRING_RECT   DEF_PAGE_STRING_RECT
#define PAGE_TAB_WIDTH     DEF_PAGE_TAB_WIDTH
#define MIN_PROP_LEFT_MARGIN DEF_MIN_PROP_LEFT_MARGIN
#define MIN_PROP_RIGHT_MARGIN DEF_MIN_PROP_RIGHT_MARGIN
#define PROP_BUTTON_RECT   DEF_PROP_BUTTON_RECT

#else

#define LINE_HEIGHT        (CProgramData::GetRealPixelsY(DEF_LINE_HEIGHT))
#define LINE_INDENT        (CProgramData::GetRealPixelsX(DEF_LINE_INDENT))
#define TEXT_HEIGHT        (CProgramData::GetRealPixelsY(DEF_TEXT_HEIGHT))
#define FOLDER_METRIC      DEF_FOLDER_METRIC
#define CONN_START         (CProgramData::GetRealPixelsX(DEF_CONN_START))
#define CONN_END           (CProgramData::GetRealPixelsX(DEF_CONN_END))
#define CONN_Y             (CProgramData::GetRealPixelsY(DEF_CONN_Y))
#define FILE_WIDTH         (CProgramData::GetRealPixelsX(DEF_FILE_WIDTH))
#define MIN_SCROLL_WIDTH   (CProgramData::GetRealPixelsX(DEF_MIN_SCROLL_WIDTH))
#define BAR_CENTER_MARGIN  (CProgramData::GetRealPixelsX(DEF_BAR_CENTER_MARGIN))
#define BAR_CENTER_SPACE   (CProgramData::GetRealPixelsX(DEF_BAR_CENTER_SPACE))

#define DEFAULT_TABWIDTH_NAME (CProgramData::GetRealPixelsX(DEF_DEFAULT_TABWIDTH_NAME))
#define DEFAULT_TABWIDTH_SIZE (CProgramData::GetRealPixelsX(DEF_DEFAULT_TABWIDTH_SIZE))
#define DEFAULT_TABWIDTH_TIME (CProgramData::GetRealPixelsX(DEF_DEFAULT_TABWIDTH_TIME))
#define MIN_TABWIDTH_NAME     (CProgramData::GetRealPixelsX(DEF_MIN_TABWIDTH_NAME))
#define MIN_TABWIDTH_OTHER    (CProgramData::GetRealPixelsX(DEF_MIN_TABWIDTH_OTHER))
#define TAB_GRAB_BIAS         (CProgramData::GetRealPixelsX(DEF_TAB_GRAB_BIAS))

#define MODIFY_DATE_PART_WIDTH (CProgramData::GetRealPixelsX(DEF_MODIFY_DATE_PART_WIDTH))
#define TABMARGIN_SIZE         (CProgramData::GetRealPixelsX(DEF_TABMARGIN_SIZE))

#define FILE_RECT          (CProgramData::GetRealRect(DEF_FILE_RECT))

#define PROP_WND_HEIGHT    (CProgramData::GetRealPixelsY(DEF_PROP_WND_HEIGHT))
#define PROP_START_X       (CProgramData::GetRealPixelsX(DEF_PROP_START_X))
#define PAGE_TAB_RECT      (CProgramData::GetRealRect(DEF_PAGE_TAB_RECT))
#define PAGE_TAB_MARGIN_SCALED (CProgramData::GetRealPixelsX(PAGE_TAB_MARGIN))
#define PAGE_STRING_RECT   (CProgramData::GetRealRect(DEF_PAGE_STRING_RECT))
#define PAGE_TAB_WIDTH     (CProgramData::GetRealPixelsX(DEF_PAGE_TAB_WIDTH))
#define MIN_PROP_LEFT_MARGIN (CProgramData::GetRealPixelsX(DEF_MIN_PROP_LEFT_MARGIN))
#define MIN_PROP_RIGHT_MARGIN (CProgramData::GetRealPixelsX(DEF_MIN_PROP_RIGHT_MARGIN))
#define PROP_BUTTON_RECT   (CProgramData::GetRealRect(DEF_PROP_BUTTON_RECT))

#endif

#define CONN_COLOR         RGB(128,128,128)
#define GREY_COLOR         RGB(248,250,249)
#define RED_COLOR          RGB(255,0,0)
#define BLUE_COLOR         RGB(0,0,255)
#define OLD_COLOR          RGB(128,128,128)
#define YELLOW_COLOR       RGB(128,128,0)
#define SEL_COLOR          RGB(204,255,204)
#define SELN_COLOR         RGB(226,246,226)
#define BACK_GREY_COLOR    RGB(160,160,160)
#define TAB_SEP_COLOR      RGB(229,229,229)

#define TAG_TYPE_DIR       "D "
#define TAG_TYPE_FILE      "N "
#define TAG_DRSIZE         "DirRecSize: "
#define TAG_SIZE           "Size: "
#define TAG_ERR_DESC       "error: "

#define dir_symbol         '\\'

#define VIEW_FONT          _T("Arial")

#endif
