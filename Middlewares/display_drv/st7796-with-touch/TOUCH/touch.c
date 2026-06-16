#include "touch.h"

_m_tp_dev tp_dev =
{
    TP_Init,
    TP_Scan,
    .touchtype = TP_TYPE_CTP,
};

//触摸按键扫描
//tp:0,屏幕坐标;1,物理坐标(校准等特殊场合用)
//返回值:当前触屏状态.
//0,触屏无触摸;1,触屏有触摸
uint8_t TP_Scan(uint8_t tp)
{
    return tp_dev.scan(tp);
}

//触摸屏初始化
//返回值:0,成功
uint8_t TP_Init(void)
{
    FT6336_Init();
    tp_dev.scan = FT6336_Scan;
    return 0;
}
