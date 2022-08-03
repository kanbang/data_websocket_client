var conf = 
{
    "WSMarketConfig": 
    {                           // 配置总名称 不用动
        "binance-f_1":          // 交易所线程  命名规则：交易所小写-衍生品类型小写_分区id
        {  
            "isstart": "1",     // 是否启动  1启动 0不启动
            "wsclass": "okx-f", // 解析类名 命名规则：交易所小写-衍生品类型小写
            "depth": 
            {                   // 订阅数据类型 depth和trade两种
                "type": "full", // depth数据类型 目前只有full
                "level": "20",  // depth数据档位 
                "symbol": [     // 品种列表
                    "BTC-USDT", // 交易品种名称 命名规则：品种名称-结算代币
                    "ETH-USDT"
                ]
            },
            "trade": 
            {                   // 订阅数据类型 depth和trade两种
                "symbol": [
                    "BTC-USDT",
                    "ETH-USDT"
                ]
            },
            "zmqstr": "tcp://127.0.0.1:5551"  //zmq发布端口
        },
        "binance-f_2":         // 交易所线程 命名规则：交易所小写-衍生品类型小写_分区id
        {
            "wsclass":"binance-f",
            "depth":
            {
                "type":"limit",
                "level":"20",
                "symbol":
                [
                    "LTC-USDT",
                    "EOS-USDT"
                ]
            },
            "trade":
            {
                "symbol":
                [
                    "LTC-USDT",
                    "EOS-USDT"
                ]
            },          
            "zmqstr":"tcp://127.0.0.1:5552"
        }
    }
}