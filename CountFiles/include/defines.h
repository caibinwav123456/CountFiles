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
#define DEF_BAR_CENTER_MARGIN  2
#define DEF_BAR_CENTER_SPACE   38

#define DEF_DEFAULT_TABWIDTH_NAME 200
#define DEF_DEFAULT_TABWIDTH_SIZE 110
#define DEF_DEFAULT_TABWIDTH_TIME 160
#define DEF_MIN_TABWIDTH_NAME     80
#define DEF_MIN_TABWIDTH_OTHER    10

#define DEF_MODIFY_DATE_PART_WIDTH 100

#define DEF_FILE_RECT          CRect(3,6,9,14)

#ifndef USE_HIDPI_SCREEN

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

#define MODIFY_DATE_PART_WIDTH DEF_MODIFY_DATE_PART_WIDTH

#define FILE_RECT          DEF_FILE_RECT

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

#define MODIFY_DATE_PART_WIDTH (CProgramData::GetRealPixelsX(DEF_MODIFY_DATE_PART_WIDTH))

#define FILE_RECT          (CProgramData::GetRealRect(DEF_FILE_RECT))

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
