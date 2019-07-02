# nRF52_GATTServer

1、创建用户服务实例、注册typedef void(* nrf_sdh_soc_evt_handler_t )(uint32_t evt_id, void *p_context)
    类型的函数（所有用户读写、连接事件全由它处理）
