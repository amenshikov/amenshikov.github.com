![](http://xsolla.com/img/xsolla-logo2.png)

## Xsolla支付API ##

===

##請按以下步驟操作：##


1. [註冊](https://account.xsolla.com/index.php?a=registrationForm "帳戶註冊") 帳戶
2. 閱讀API
   * [虛擬貨幣](https://github.com/xsolla/Xsolla-Payment-API/blob/master/documentation/chinese/Xsolla_Virtual_Currency_API_Guide_Chinese.pdf "虛擬貨幣協定API指南")
   * [現金協議](https://github.com/xsolla/Xsolla-Payment-API/blob/master/documentation/chinese/Xsolla_Cash_API_Guide_Chinese.pdf "現金協定API指南")
3. [添加新項目](https://account.xsolla.com/index.php?a=projects&ext=drawfrmnewproject "添加項目") 至帳戶
4. 閱讀[PayBar](https://github.com/xsolla/Xsolla-Payment-API/blob/master/documentation/chinese/Xsolla_PayBar_Integration_Guide_Chinese.pdf "PayBar集成指南") / [Paystation](https://github.com/xsolla/Xsolla-Payment-API/blob/master/documentation/chinese/Xsolla_PayStation_Integration_Guide_Chinese.pdf "PayStation集成指南") 指南並實現其中一種工具。
若想對工具進行定制，參見[範本檔](https://github.com/xsolla/Xsolla-Payment-API/blob/master/Paystation_template.zip "Paystation範本檔").
5. 測試並上線運行。


##虛擬貨幣協議##

Xsolla的虛擬貨幣協議允許按預先設置的匯率將真實貨幣兌換為虛擬貨幣。對於擁有設有預定值的遊戲內虛擬貨幣的專案而言，虛擬貨幣協議是一種簡單且適用的解決方案。當用戶為其在遊戲中的帳戶充值時，用戶將收到預先設定的虛擬貨幣金額。玩家可通過電子錢包、現金亭、手機、網上銀行等進行支付。

####實施虛擬貨幣協議####
實施Xsolla的虛擬貨幣協定步驟簡單，只需編輯內含檔[config.php](https://github.com/xsolla/Xsolla-Payment-API/blob/master/examples/virtual_currency_protocol/inc/config.php "配置.php")，使之包含資料庫資訊和金鑰。
擴展[虛擬貨幣.php](https://github.com/xsolla/Xsolla-Payment-API/blob/master/examples/virtual_currency_protocol/inc/virtual_currency_protocol.php "虛擬貨幣.php")中的內含虛擬貨幣類別並實施以下方法進行資料庫處理：

* **設置DB（）**
    * 本方法通過例示PDO物件，為資料庫配置連接
* **用戶存在（$用戶）**
    * 本方法用來檢查資料庫是否存在使用者，若發現用戶，則返回布林真值，反之返回假值
* **發票存在（$發票ID）**
    * 本方法用來檢查資料庫是否存在發票，若發現發票，則返回布林真值，反之返回假值
* **新發票（$發票ID，$用戶ID，$金額）**
    * 本方法向資料庫中插入新發票 
* **取消發票（$發票ID）**
    * 本方法從資料庫中刪除發票

若對如何實施以上方法存在任何疑問，請參見內含檔[示例.php](https://github.com/xsolla/Xsolla-Payment-API/blob/master/examples/virtual_currency_protocol/example.php "示例.php")，該檔使用了[示例.sql](https://github.com/xsolla/Xsolla-Payment-API/blob/master/examples/virtual_currency_protocol/example.sql "示例.sql")中的資料庫結構。


##現金協議##
Xsolla的現金協議允許遊戲專案銷售大量虛擬商品和服務。使用本協定時，遊戲專案中形成一個訂單。


*關於協定的更多資訊，請訪問網站[http://xsolla.com/docs/section/protocols](http://xsolla.com/docs/section/protocols "協定更多資訊")*

##其他資源##
*若需要任何説明，請[聯繫我們](發郵件至：a.menshikov@xsolla.com "集成經理").*
*如果您发现了一个问题，或者需要创建新的API，请添加您的请求[这里]（https://github.com/xsolla/Xsolla-Payment-API/issues)*

**-Xsolla團隊** 
