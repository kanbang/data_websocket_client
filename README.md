
##程序介绍

1.  从config文件读取配置信息
2.  从指定网站订阅websocket数据
3.  将数据统一为标准的json格式
4.  将数据转发到zmq端口

##开发环境

vs2019
windows10
windows server 2019


##开发依赖

1. json 解析库  使用 rapidjson (开源的三方库，只需包含头文件即可，所有头文件在 rapidjson_include.rar)

2. https websocket 通讯使用 libhv(开源的三方库，项目在github上)
https://github.com/ithewei/libhv/blob/master/README-CN.md

3. zmq 组件 libzmq-4.3.4，其实版本不限制
https://zeromq.org/get-started/?language=cpp&library=zmqpp#

4. 不使用 rapidjson libhv  zmq  openssl 之外的其他三方库

5. 原工程可编译运行，模式是 64位 release 模式

##配置文件说明

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

##数据格式
trades：

{
    "instrument_id": "binance-f_BTC-USDT_trades", #名称
    "data_type": "trades-feed",  #数据类型
    "exchange": "BINANCE_FUTURES", #交易所——品种类型名称
    "symbol": "BTC-USDT-PERP", #品种名称
    "side": "buy", #买卖方向
    "amount": 0.1, #成交数量
    "price": 47412.3, #成交金额
    "id": "1154117591", #成交ID
    "type": None, #成交类型
    "timestamp": 1648623478.337, #成交时间戳
    "receipt_timestamp": 1648623478.5280075 #本地接受数据时间戳
}

orderbook：

{
    "instrument_id": "binance-f_BTC-USDT_depth-10",
    "data_type": "depth-10-feed",
    "exchange": "BINANCE_FUTURES",
    "symbol": "BTC-USDT-PERP",
    "book": {
        "bid": {
            "47412.2": 4.459,
            "47412.1": 0.001,
            "47411.6": 0.5,
            "47411.2": 2,
            "47409.5": 0.05,
            "47409.4": 0.097,
            "47409.3": 4.874,
            "47408.9": 0.011,
            "47408.8": 0.838,
            "47408.7": 4.462
        },
        "ask": {
            "47412.3": 0.017,
            "47412.7": 0.022,
            "47412.8": 0.117,
            "47412.9": 0.117,
            "47413.1": 0.001,
            "47413.4": 0.117,
            "47414.1": 0.005,
            "47415.7": 0.008,
            "47415.9": 0.065,
            "47416": 0.189
        }
    },
    "timestamp": 1648623478.583, #成交时间戳
    "receipt_timestamp": 1648623478.6130261 #本地接受数据时间戳
}