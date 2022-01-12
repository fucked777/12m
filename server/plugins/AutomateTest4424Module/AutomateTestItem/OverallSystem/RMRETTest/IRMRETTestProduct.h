#ifndef IRMRETTESTPRODUCT_H
#define IRMRETTESTPRODUCT_H

#include "SingleCommandHelper.h"

class IRMRETTestProduct
{
public:
    virtual bool setLoop() = 0;                           //设置环路
    virtual bool setBaseCmd() = 0;                        //调整基带参数
    virtual bool setXLReduce() = 0;                       //校零衰减调整
    virtual bool startTest(CmdResult& result) = 0;        //开始测试
    virtual bool saveResult(const CmdResult result) = 0;  //保存结果
};

#endif  // IRMRETTESTPRODUCT_H
