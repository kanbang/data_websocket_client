
程序介绍

1.  从config文件读取配置信息
2.  从指定网站订阅websocket数据
3.  将数据统一为标准的json格式
4.  将数据转发到zmq端口


开发依赖

1. json 解析库  使用 rapidjson (开源的三方库，只需包含头文件即可，所有头文件在 rapidjson_include.rar)

2. https websocket 通讯使用 libhv(开源的三方库，项目在github上)
https://github.com/ithewei/libhv/blob/master/README-CN.md

3. zmq 组件 libzmq-4.3.4，其实版本不限制
https://zeromq.org/get-started/?language=cpp&library=zmqpp#






配置文件说明

一、启动流程
1.  编辑 myconfig.json 配置文件
2.  双击 myplatform_market.exe 启动程序

二、配置文件说明

以下面json内容为例

1.  "WSMarketConfig"的每个子元素 表示一个 websocket 连接通道，一个连接通道对应一个发送端口（zmqstr指定）

2.  子元素（如 "binance-f_1"） 中，
    1） "wsclass" 表示 交易所名称  （如binance-f,okx-f）
    2） "depth" 表示 订阅深度信息
    3） "trade" 表示 订阅成交信息
    4） "zmqstr" 行情发送位置

3.  "depth" 中  
    1） "type":"limit" 表示有限档深度行情，"full" 表示增量深度行情
    2） "level":"20" 表示 取20档，即只 发送前20档数据到 zmq
    3） "symbol" 指 订阅哪些品种的行情

三、注意
如果需要同时订阅全品种信息，考虑到python的接收效率 和 交易所单连接可支持的最大订阅数量，建议 分成若干个 连接通道，如下（"binance-f_1"， "binance-f_2"）

{
    "WSMarketConfig":
    {
        "binance-f_1":
        {
            "wsclass":"binance-f",
            "depth":
            {
                "type":"limit",
                "level":"20",
                "symbol":["BTC-USDT","OP-USDT","GAL-USDT","DAR-USDT","JASMY-USDT","FTT-USDT","WOO-USDT","BNX-USDT"]
            },
            "trade":
            {
                "symbol":["BTC-USDT","OP-USDT","GAL-USDT","DAR-USDT","JASMY-USDT","FTT-USDT","WOO-USDT","BNX-USDT"]
            },          
            "zmqstr":"tcp://127.0.0.1:5551"
        },
        "binance-f_2":
        {
            "wsclass":"binance-f",
            "depth":
            {
                "type":"limit",
                "level":"20",
                "symbol":["CTK-USDT","BEL-USDT","CVC-USDT","OCEAN-USDT","MATIC-USDT","LRC-USDT","RSR-USDT"]
            },
            "trade":
            {
                "symbol":["CTK-USDT","BEL-USDT","CVC-USDT","OCEAN-USDT","MATIC-USDT","LRC-USDT","RSR-USDT"]
            },          
            "zmqstr":"tcp://127.0.0.1:5552"
        }
    }
}
