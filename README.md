# esp_llm_chat
一个将esp32s3接入大部分大语言模型的Demo。
需要自己配置大模型的访问URL和密钥KEY，智谱清言有免费API，可以去它们官网申请自己用。
如果遇到证书问题，menuconfig->ESP-TLS里面最后一项和安全有关的配置启用，然后里面新出来的一项启用。
注意，所有s3项目均使用N16R8最大带宽速度spi+240M最大速度进行配置。
请确保你的闪存和内存够用，由于非工业使用，没有专门在极限情况下进行测试，理想情况下它们应该均工作正常。
![467fddc4abe956027c494dd6beba1060](https://github.com/user-attachments/assets/1319030d-b285-4c09-b04d-aa86d9e221fe)
