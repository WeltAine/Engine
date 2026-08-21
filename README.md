# Engine
游戏引擎（期望）

## System 生命周期使用约定

Pipeline、配置和执行计划的构建错误会在 World 停止旧 Schedule 前报告，
因此这类错误不会替换旧 Schedule。

`OnAttach`、`OnBegin`、`OnEnd` 和 `OnDetach` 属于运行时生命周期副作用。
如果 System 在这些回调中抛出异常，Schedule 只负责记录错误并尽力清理，
不保证完整恢复旧 Schedule，也不保证已经发生的外部副作用可以回滚。

System 生命周期回调应尽量自行处理错误，不依赖异常实现正常控制流。
