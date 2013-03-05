![](http://xsolla.com/img/xsolla-logo2.png)

## Xsolla 결제 API ##

===

## 다음 단계를 따라하세요: ##


1. [등록](https://account.xsolla.com/index.php?a=registrationForm "계정 등록") 회원님의 계정
2. API 정보를 참조하세요
   * [Virtual Currency](https://github.com/xsolla/Xsolla-Payment-API/blob/master/documentation/korean/Xsolla_Virtual_Currency_API_Guide_Korean.pdf "Virtual Currency 프로토콜 API 가이드")
   * [Cash protocol](https://github.com/xsolla/Xsolla-Payment-API/blob/master/documentation/korean/Xsolla_Virtual_Currency_API_Guide_Korean.pdf "Cash 프로토콜 API 가이드")
3. [신규 프로젝트 추가](https://account.xsolla.com/index.php?a=projects&ext=drawfrmnewproject "프로젝트 추가") 회원님의 계정에 추가됩니다
4. 참조 [PayBar](https://github.com/xsolla/Xsolla-Payment-API/blob/master/documentation/korean/Xsolla_PayBar_Integration_Guide_Korean.pdf "PayBar Integration Guide") / [Paystation](https://github.com/xsolla/Xsolla-Payment-API/blob/master/documentation/korean/Xsolla_PayStation_Integration_Guide_Korean.pdf "PayStation 인티그레이션 가이드") 가이드와 다음 도구 중 하나를 만드십시오. 
커스터마이징을 하기 원하는 경우 다음 템플릿 파일을 참조하세요 [template files](https://github.com/xsolla/Xsolla-Payment-API/blob/master/Paystation_template.zip "Paystation template files").
5. 테스트와 실행


## Virtual Currency 프로토콜 ##

Xsolla Virtual Currency 프로토콜은 실제 통화를 사전 설정된 환전율로 Virtual Currency로 환전하도록 해줍니다. Virtual Currency 프로토콜은 특히 사전에 가격이 정해져 있는 인게임 virtual currency로 사용하는 프로젝트를 위해 쉽고 용이하게 사용할수 있습니다. 고객이 인게임 계정에 충전할 때 사전에 설정된 virtual currency을 구매할 수 있습니다. 플레이어는 e-wallet, 결제 키오스크, 모바일, 온라인 뱅킹등의 방식을 사용하여 결제할 수 있습니다.

## Virtual Currency 프로토콜 ##
Xsolla's Virtual Currency 프로토콜은 데이터 베이스와 secret key를 포함한 [config.php](https://github.com/xsolla/Xsolla-Payment-API/blob/master/examples/virtual_currency_protocol/inc/config.php "config.php") 파일을 편집하여 쉽게 수정할 수 있습니다. 단수하게 [VirtualCurrency.php](https://github.com/xsolla/Xsolla-Payment-API/blob/master/examples/virtual_currency_protocol/inc/virtual_currency_protocol.php "VirtualCurrency.php") 파일에 있는 VirtualCurrency 클래스를 확장하고 다음과 같은 방식으로 데이터 베이스를 처리합니다:

* **setupDB()**
    * 이 함수로 PDO 객체를 초기화하고 데이터베이스와 연결합니다
* **userExists($user)**
    * 이 함수로 고객이 데이터 베이스에 존재하는지 체크하고 존재 여부를 true/false 값 형태로 리턴합니다
* **invoiceExists($invoiceID)**
    * 이 함수로 고객의 인보이스가 존재하는지 체크하고 존재 여부를 true/false 값 형태로 리턴합니다
* **newInvoice($invoiceID, $userID, $sum)**
    이 함수로 새 인보이스를 데이터 베이스에 추가합니다 
* **invoiceExists($invoiceID)**
    이 함수를 호출하여 데이터 베이스에 있는 인보이스를 삭제합니다

이 함수를 제작하는 방법에 대해 문의하려면 [example.php](https://github.com/xsolla/Xsolla-Payment-API/blob/master/examples/virtual_currency_protocol/example.php "example.php") 내용을 확인하세요. 이 코드는 [example.sql]에 기술된 데이터 베이스 를 처리할 수 있습니다(https://github.com/xsolla/Xsolla-Payment-API/blob/master/examples/virtual_currency_protocol/example.sql "example.sql").


## Cash Protocol ##
Xsolla 캐시 프로토콜은 게임 프로젝트가 가상 상품과 서비스 팩을 판매할 수 있는 기능을 제공합니다. 이 프로토콜을 사용할 때 게임 프로젝트 상에서 주문을 직접 할 수 있습니다. 


*보다 더 상세한 정보는 다음 사이트를 참조하세요 [http://xsolla.com/docs/section/protocols](http://xsolla.com/docs/section/protocols "More about protocols")*

## 추가 정보 ##
*도움이 필요하시면 [연락처](mailto: a.menshikov@xsolla.com "Integration manager").*

**-Xsolla 팀** 
