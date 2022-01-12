
#ifndef CONTROLATTRIBUTE_H
#define CONTROLATTRIBUTE_H

#define ControlLabelWidth 150
#define ControlValueWidth 140

#define ControlStatusLEDWidth  16
#define ControlStatusLEDHeight 16
//#define ControlWidget ControlLabelWidth + ControlValueWidth + ControlUnitLabelWidth + 3 * ControlSpace  //控件宽度

#define InputControlSpace     10
#define StatusputControlSpace 5
/* 此宽度是输入数据的 */
#define ControlWidgetI    ControlLabelWidth + ControlValueWidth + InputControlSpace          //控件宽度
#define ControlWidgetS    ControlLabelWidth + ControlValueWidth + StatusputControlSpace * 2  //控件宽度
#define ControlHeight     25                                                                 //控件高度
#define UnitExtendedWidth 5                                                                  // 单位的扩展宽度

/* 单位单字符宽度 */
//#define ControlUnitSingleCharacterW 10
//#define ControlUnitLabelCharactersNum 4

#endif  // CONTROLATTRIBUTE_H
