![](http://xsolla.com/img/xsolla-logo2.png)

## XSOLLA Payment API ##

===
XSOLLA is an international payment solution provider specializing in online game payments.
See our profile: http://xsolla.com/xsolla_profile.pdf
## Please follow these easy steps: ##


1. [Register](https://account.xsolla.com/index.php?a=registrationForm "Account registration") your account
2. Read our API [Virtual Currency Wiki](https://github.com/xsolla/Xsolla-Payment-API/wiki/Virtual-Currency-API-Guide "Virtual Currency API Wiki") and [Cash protocol Wiki](https://github.com/xsolla/Xsolla-Payment-API/wiki/Cash-API-Guide "Cash Protocol API Wiki") or just print it:
   * [Virtual Currency](https://github.com/xsolla/Xsolla-Payment-API/blob/master/documentation/english/Xsolla_Virtual_Currency_API_Guide.pdf "Virtual Currency Protocol API Guide")
   * [Cash protocol](https://github.com/xsolla/Xsolla-Payment-API/blob/master/documentation/english/Xsolla_Cash_API_Guide.pdf "Cash Protocol API Guide")
3. [Add a new project](https://account.xsolla.com/index.php?a=projects&ext=drawfrmnewproject "Add project") to your account
4. Read [PayBar](hhttps://github.com/xsolla/Xsolla-Payment-API/blob/master/documentation/english/Xsolla_PayBar_Integration_Guide_en.pdf "PayBar Integration Guide") / [Paystation](https://github.com/xsolla/Xsolla-Payment-API/blob/master/documentation/english/Xsolla_PayStation_Integration_Guide.pdf "PayStation Integration Guide") guides and implement one of these tools. 
In case you would like to customize it, here is the [template files](https://github.com/xsolla/Xsolla-Payment-API/blob/master/Paystation_template.zip "Paystation template files").
5. Test and go live.

#Paystation#
![](https://www.evernote.com/shard/s132/sh/1c148183-aaf8-40e8-8db5-ce41797f06ca/72e98388f44a9d09427a11afbc7e0b46/res/59d278a3-7373-492f-9588-ffd5e7a98547/skitch.png)

#Credit cards form + PayBar#
![](https://www.evernote.com/shard/s132/sh/137a980a-d1bc-473c-a7dc-442b3d6566ce/3681fd65a158a3a86c42899d91e3cdf6/res/032a8fef-d9dc-447c-b94f-f29285a0a828/skitch.png)

#Mobile Paystation#
![](https://www.evernote.com/shard/s132/sh/3016cdae-965b-4530-a23b-30163c07151b/b8bf1924cd1c4f2a96e6dfa02e47375f/res/4b64daeb-26af-4282-bee4-f077c229aa2a/skitch.png)

## Virtual Currency Protocol ##

Xsolla's Virtual Currency Protocol allows the exchange of real currency into virtual currency with a preset exchange rate. The Virtual Currency Protocol is an easy and accessible solution for those projects which have in-game virtual currency with a predetermined value. Users get a preset amount of virtual currency when they replenish their accounts in-game. Players can make payments from e-wallets, cash kiosks, mobile, online-banking, etc.

#### Implementing Virtual Currency Protocol ####
Implementing Xsolla's Virtual Currency Protocol is as easy as editing the included [config.php](https://github.com/xsolla/Xsolla-Payment-API/blob/master/examples/virtual_currency_protocol/inc/config.php "config.php") to include your database information and secret key. Simply extend the included VirtualCurrency class found in [VirtualCurrency.php](https://github.com/xsolla/Xsolla-Payment-API/blob/master/examples/virtual_currency_protocol/inc/virtual_currency_protocol.php "VirtualCurrency.php") and implement the following methods for database handling:

* **setupDB()**
    * this method is responsible for configuring a connection to your database by instantiating a PDO object
* **userExists($user)**
    * this method checks the database for a user and returns a boolean true if found or false otherwise
* **invoiceExists($invoiceID)**
    * this method checks the database for the existence of an invoice and returns boolean true if found or false otherwise
* **newInvoice($invoiceID, $userID, $sum)**
    * this method inserts a new invoice into your database 
* **cancelInvoice($invoiceID)**
    * this method deletes an invoice from your database

if you have questions about how to implement these methods please see the included [example.php](https://github.com/xsolla/Xsolla-Payment-API/blob/master/examples/virtual_currency_protocol/example.php "example.php") which utilizes the database structure found in [example.sql](https://github.com/xsolla/Xsolla-Payment-API/blob/master/examples/virtual_currency_protocol/example.sql "example.sql").
We also have other samples:
* [ASP/VB Sample](https://github.com/xsolla/Xsolla-Payment-API/blob/master/examples/virtual_currency_protocol/standart.protocol.implementation_en_asp.asp)
* [ASP.NET/C# Sample](https://github.com/xsolla/Xsolla-Payment-API/blob/master/examples/virtual_currency_protocol/standart.protocol.implementation_en_aspx.aspx.cs)
* [C Sample](https://github.com/xsolla/Xsolla-Payment-API/blob/master/examples/virtual_currency_protocol/standart.protocol.implementation_en_c.c)
* [C++ Sample](https://github.com/xsolla/Xsolla-Payment-API/blob/master/examples/virtual_currency_protocol/standart.protocol.implementation_en_cpp.h)
* [Python Sample](https://github.com/xsolla/Xsolla-Payment-API/blob/master/examples/virtual_currency_protocol/standart.protocol.implementation_en_python.py)
* [Ruby Sample](https://github.com/xsolla/Xsolla-Payment-API/blob/master/examples/virtual_currency_protocol/standart.protocol.implementation_en_ruby.ruby)

## Cash Protocol ##
Xsolla's Cash Protocol enables game projects to sell packs of virtual goods and services. When using this protocol, an order is made on the side of the game project. 


*For additional information about protocols, please visit [http://xsolla.com/docs/section/protocols](http://xsolla.com/docs/section/protocols "More about protocols")*

## Additional resources ##
*If you need any help please [contact us](mailto: a.menshikov@xsolla.com "Integration manager").*
*If you found an issue or need to create new API, please add your request [here](https://github.com/xsolla/Xsolla-Payment-API/issues)*

**-Xsolla Team** 
[![githalytics.com alpha](https://cruel-carlota.pagodabox.com/83459fc49878adb201efdb4ec58a5f92 "githalytics.com")](http://githalytics.com/xsolla/Xsolla-Payment-API)
