// Virtual Currency Protocol Implementation. C++ Sample.
//
// @version 1.0
// @author Xsolla


#include <string>
#include <sstream>
#include <iostream>

#include "mysql.h"
#include "openssl/md5.h"


using namespace std;


// Config class
class XsollaConfig
{
public:
	// Parameters for database connection
	// @var std::string
	string							m_dbHost;
	string							m_dbUser;
	string							m_dbPassword;
	string							m_db;

	// Table that contains the information about the payments
	// @var std::string
	string							m_dbPaymentsTable;

	// Table where you store character's names or accounts
	// @var std::string
	string							m_dbCharactersTable;

	// Secret key for your project, you can ask for it your account manager
	// @var std::string
	string							m_SecretKey;

	// List of allowed IP
	// @var int
	int								m_NumAllowedIPs;
	// @var std::string *
	string *						m_AllowedIPs;
};


// Class that implements "Virtual Currency Protocol"
//
// In this sample class you can find implementation of main methods of "Virtual Currency Protocol".
// To start script, replace parameters by your own parameters (based on your system).
//
// @version 1.0
// @author Xsolla
class VirtualCurrencyProtocolImplementation
{
private:
	XsollaConfig					m_Config;
	MYSQL *							m_Connect;

public:
	// Result codes
	enum
	{
		SUCCESS = 0,
		TEMPORARY_ERROR,
		CANCEL_NOT_FOUND,
		INCORRECT_SIGNATURE,
		INCORRECT_REQUEST_FORMAT,
		OTHER_ERROR,
		PAYMENT_CANNOT_BE_PROCESSED
	};

	// Instantiates the VirtualCurrencyProtocolImplementation class
	// @throws std:string
	VirtualCurrencyProtocolImplementation::VirtualCurrencyProtocolImplementation(XsollaConfig Config, const string strRemoteAddress, const string strCommand, const string strId, const string strSum, const string strV1, const string strV2, const string strV3, const string strMD5)
	{
		m_Config = Config;
		m_Connect = mysql_init(NULL);
		if(m_Connect != NULL)
			m_Connect = mysql_real_connect(m_Connect, Config.m_dbHost.c_str(), Config.m_dbUser.c_str(), Config.m_dbPassword.c_str(), Config.m_db.c_str(), 0, NULL, 0);
		if(m_Connect != NULL)
			ProcessRequest(strRemoteAddress, strCommand, strId, strSum, strV1, strV2, strV3, strMD5);
		else
			ErrorCheckResponse("Unable to connect to database");
	}

	VirtualCurrencyProtocolImplementation::~VirtualCurrencyProtocolImplementation(void)
	{
		if(m_Connect != NULL)
			mysql_close(m_Connect);
	}

private:
	// Checks signature
	// @param std::string strSecretKey
	// @param std::string strMD5
	// @return bool
	bool CheckSignature(const string strSecretKey, const string strMD5)
	{
		unsigned char					pResult[MD5_DIGEST_LENGTH];
		string							strResult;

		MD5((const unsigned char *) strSecretKey.c_str(), strSecretKey.length(), pResult);

		for(int i=0; i<MD5_DIGEST_LENGTH; i++)
		{
			char							pBuf[8];

			sprintf(pBuf, "%02X", pResult[i]);
			strResult += pBuf;
		}

		if(strMD5.compare(strResult) != 0)
			return(false);

		return(true);
	}

	// Method for nickname check
	// @param std::string strV1
	// @param std::string strV2
	// @param std::string strV3
	// @param std::string strMD5
	// @throws std::string
	void ProcessCheckRequest(const string strV1, const string strV2, const string strV3, const string strMD5)
	{
		// Checking existance of v1 - nickname, account and so on
		if(strV1.length() <= 0)
			throw string("User ID is undefined");

		// Checking existance of v2, v3. If you don't support these parametres, please comment out these lines
		if(strV2.length() <= 0)
			throw string("User ID is undefined");

		if(strV3.length() <= 0)
			throw string("User ID is undefined");

		// Checking existance of md5 param
		if(strMD5.length() <= 0)
			throw string("Signature is undefined");

		// Checking signature
		if(CheckSignature(string("check") + strV1 + m_Config.m_SecretKey, strMD5) == false)
			throw string("Incorrect signature");

		try
		{
			int								iCode;
			string							strDescription;

			Check(strV1, strV2, strV3, iCode, strDescription);

			// Generating response
			GenerateCheckResponse(iCode, strDescription);
		}
		catch(string strMsg)
		{
			ErrorCheckResponse(strMsg);
		}
	}

	// Pay method
	// @param std::string strId
	// @param std::string strSum
	// @param std::string strV1
	// @param std::string strV2
	// @param std::string strV3
	// @param std::string strMD5
	// @throws std::string
	void ProcessPayRequest(const string strId, const string strSum, const string strV1, const string strV2, const string strV3, const string strMD5)
	{
		// Neccessary checking
		if(strId.length() <= 0)
			throw string("Invoice is undefined");

		if(strV1.length() <= 0)
			throw string("User ID is undefined");

		// If you don't support these parameters, please comment out these lines
		if(strV2.length() <= 0)
			throw string("User ID is undefined");

		if(strV3.length() <= 0)
			throw string("User ID is undefined");

		if(strSum.length() <= 0)
			throw string("Amount is undefined");

		if(strMD5.length() <= 0)
			throw string("Signature is undefined");

		if(CheckSignature(string("pay") + strV1 + strId + m_Config.m_SecretKey, strMD5) == false)
			throw string("Incorrect signature");

		try
		{
			int								iCode;
			string							strDescription;
			string							strIdShop;

			Pay(strId, strSum, strV1, strV2, strV3, iCode, strDescription, strIdShop);
			GeneratePayResponse(iCode, strDescription, strId, strIdShop, strSum);
		}
		catch(string strMsg)
		{
			ErrorPayResponse(strMsg);
		}
	}

	// Payment cancel method
	// @param std::string strId
	// @param std::string strMD5
	// @throws Exception
	void ProcessCancelRequest(const string strId, const string strMD5)
	{
		if(strId.length() <= 0)
			throw string("Invoice is undefined");

		if(strMD5.length() <= 0)
			throw string("Signature is undefined");

		if(CheckSignature(string("cancel") + strId + m_Config.m_SecretKey, strMD5) == false)
			throw string("Incorrect signature");

		try
		{
			int								iCode;
			string							strDescription;

			Cancel(strId, iCode, strDescription);

			// Generate response
			GenerateCancelResponse(iCode, strDescription);
		}
		catch(string strMsg)
		{
			ErrorCancelResponse(strMsg);
		}
	}

	// Main method
	// @param std::string strRemoteAddress
	// @param std::string strCommand
	// @param std::string strId
	// @param std::string strSum
	// @param std::string strV1
	// @param std::string strV2
	// @param std::string strV3
	// @param std::string strMD5
	void ProcessRequest(const string strRemoteAddress, const string strCommand, const string strId, const string strSum, const string strV1, const string strV2, const string strV3, const string strMD5)
	{
		try
		{
			int								i;

			// Checking the IP-address
			for(i=0; i<m_Config.m_NumAllowedIPs; i++)
			{
				if(strRemoteAddress.compare(m_Config.m_AllowedIPs[i]) == 0)
					break;
			}
			if(i >= m_Config.m_NumAllowedIPs)
				throw string("IP address is not allowed");

			if(strCommand.length() <= 0)
				throw string("Command is undefined");

			if(strCommand.compare("check") == 0)
				ProcessCheckRequest(strV1, strV2, strV3, strMD5);
			else if(strCommand.compare("pay") == 0)
				ProcessPayRequest(strId, strSum, strV1, strV2, strV3, strMD5);
			else if(strCommand.compare("cancel") == 0)
				ProcessCancelRequest(strId, strMD5);
			else
				throw string("Incorrect command");
		}
		catch(string strMsg)
		{
			ErrorCheckResponse(strMsg);
		}
	}

	// Generating response when using nickname check method
	// @param int iCode
	// @param std::string strDescription
	void GenerateCheckResponse(int iCode, const string strDescription)
	{
		string							strXML;

		strXML = "<?xml version=\"1.0\"?>";
		strXML += "<response>";
		strXML += string("<result>") + static_cast<ostringstream*>(&(ostringstream() << iCode))->str() + string("</result>");
		strXML += string("<comment>") + strDescription + string("</comment>");
		strXML += "</response>";

		cout << "Content-Type: text/xml; charset=utf-8\r\n\r\n";
		cout << strXML;
	}

	// Generating response when using pay method
	// @param int iCode
	// @param std::string strDescription
	// @param std::string strInvoice
	// @param std::string strOrder
	// @param std::string strSum
	void GeneratePayResponse(int iCode, const string strDescription, const string strInvoice, const string strOrder, const string strSum)
	{
		string							strXML;

		strXML = "<?xml version=\"1.0\"?>";
		strXML += "<response>";
		strXML += string("<id>") + strInvoice + string("</id>");
		strXML += string("<id_shop>") + strOrder + string("</id_shop>");
		strXML += string("<sum>") + strSum + string("</sum>");
		strXML += string("<result>") + static_cast<ostringstream*>(&(ostringstream() << iCode))->str() + string("</result>");
		strXML += string("<comment>") + strDescription + string("</comment>");
		strXML += "</response>";

		cout << "Content-Type: text/xml; charset=utf-8\r\n\r\n";
		cout << strXML;
	}

	// Generating response when using payment cancel method
	// @param int iCode
	// @param std::string strDescription
	void GenerateCancelResponse(int iCode, const string strDescription)
	{
		string							strXML;

		strXML = "<?xml version=\"1.0\"?>";
		strXML += "<response>";
		strXML += string("<result>") + static_cast<ostringstream*>(&(ostringstream() << iCode))->str() + string("</result>");
		strXML += string("<comment>") + strDescription + string("</comment>");
		strXML += "</response>";

		cout << "Content-Type: text/xml; charset=utf-8\r\n\r\n";
		cout << strXML;
	}


	void ErrorCheckResponse(const string strMessage)
	{
		GenerateCheckResponse(PAYMENT_CANNOT_BE_PROCESSED, strMessage);
	}

	void ErrorPayResponse(const string strMessage)
	{
		GeneratePayResponse(PAYMENT_CANNOT_BE_PROCESSED, strMessage, "", "", "");
	}

	void ErrorCancelResponse(const string strMessage)
	{
		GenerateCancelResponse(PAYMENT_CANNOT_BE_PROCESSED, strMessage);
	}

	//////////////////////////////////////////////////////////////////////////
	////// THIS SECTION IS THE ONLY PLACE WHERE YOU SHOULD WRITE CODE ////////
	//////////////////////////////////////////////////////////////////////////
	string AddSlashes(const string strFrom)
	{
		char *							pTo;
		string							strTo;

		pTo = new char[(strFrom.length() * 2) + 1];
		mysql_real_escape_string(m_Connect, pTo, strFrom.c_str(), strFrom.length());
		strTo = pTo;
		delete [] pTo;

		return(strTo);
	}

	// YOUR CHECK CODE HERE
	void Check(const string strV1, const string strV2, const string strV3, int & iCode, string & strDescription)
	{
		string							strSQL;
		MYSQL_RES *						pResult;
		MYSQL_ROW						pRow;

		// Code example

		// If you don't use v2, v3, use the string below
		//strSQL = string("SELECT count(1) as cnt FROM ") + m_XsollaConfig.m_dbCharactersTable + string(" WHERE v1 = ") + AddSlashes(strV1);
		strSQL = string("SELECT count(1) as cnt FROM ") + m_Config.m_dbCharactersTable + string(" WHERE v1 = ") + AddSlashes(strV1) + string(" AND v2 = ") + AddSlashes(strV2) + string(" AND v3 = ") + AddSlashes(strV3);
		// performing query
		if(mysql_query(m_Connect, strSQL.c_str()) != 0)
			throw string("Failed to query database");
		// getting result
		pResult = mysql_store_result(m_Connect);
		if(pResult == NULL)
			throw string("Failed to query database");
		pRow = mysql_fetch_row(pResult);
		if(pRow == NULL)
			throw string("Failed to query database");
		// if nickname exists
		if(string(pRow[0]).length() > 0)
		{
			iCode = SUCCESS;
			strDescription = "OK";
		}
		else
		{
			iCode = PAYMENT_CANNOT_BE_PROCESSED;
			strDescription = "Character doesn't exist.";
		}
		mysql_free_result(pResult);
	}

	// YOUR PAY CODE HERE
	void Pay(const string strId, const string strSum, const string strV1, const string strV2, const string strV3, int & iCode, string & strDescription, string & strIdShop)
	{
		string							strSQL;
		MYSQL_RES *						pResult;
		my_ulonglong					iCount;

		// Code example

		// Looking for payment with such id
		strSQL = string("SELECT `id` FROM ") + m_Config.m_dbPaymentsTable  + string(" WHERE `invoice` = ") + strId;
		// performing query
		if(mysql_query(m_Connect, strSQL.c_str()) != 0)
			throw string("Failed to query database");
		// getting result
		pResult = mysql_store_result(m_Connect);
		if(pResult == NULL)
			throw string("Failed to query database");
		// If there is no payment with this id, insert it
		iCount = mysql_num_rows(pResult);
		if(iCount == 0)
		{
			my_ulonglong					iIdShop;

			// If you don't use v2, v3, use the string below
			//strSQL = string("INSERT INTO `") + m_Config.m_dbPaymentsTable + string("` (`v1`, `amount`, `invoice`, `date_add`, `canceled`) VALUES (") + strV1 + string(", ") + strSum + string(", ") + strId + string(", NOW(), \"0\")");
			strSQL = string("INSERT INTO `") + m_Config.m_dbPaymentsTable + string("` (`v1`, `v2`, `v3`, `amount`, `invoice`, `date_add`, `canceled`) VALUES (") + strV1 + string(", ") + strV2 + string(", ") + strV3 + string(", ") + strSum + string(", ") + strId + string(", NOW(), \"0\")");
			// performing query
			if(mysql_query(m_Connect, strSQL.c_str()) != 0)
				throw string("Failed to query database");
			// if insert successful
			iIdShop = mysql_insert_id(m_Connect);
			if(iIdShop != 0)
			{
				iCode = SUCCESS;
				strDescription = "OK";
				strIdShop = static_cast<ostringstream*>(&(ostringstream() << iIdShop))->str();
			}
			else
			{
				iCode = OTHER_ERROR;
				strDescription = "Couldn't insert data. See mysql errors";
				strIdShop = "";
			}
		}
		else
		{
			MYSQL_ROW						pRow;

			// If payment with such xsolla id exists, successful
			pRow = mysql_fetch_row(pResult);
			if(pRow == NULL)
				throw string("Failed to query database");
			iCode = SUCCESS;
			strDescription = "OK";
			strIdShop = pRow[0];
		}
		mysql_free_result(pResult);
	}

	// YOUR CANCEL CODE HERE
	void Cancel(const string strId, int & iCode, string & strDescription)
	{
		string							strSQL;

		// Code example

		// Canceling payment
		strSQL = string("UPDATE `") + m_Config.m_dbPaymentsTable + string("` SET `canceled` = \"1\", `date_cancel` = NOW() WHERE `invoice` = ") + strId;
		// performing query
		if(mysql_query(m_Connect, strSQL.c_str()) == 0)
		{
			iCode = SUCCESS;
			strDescription = "OK";
		}
		else
		{
			iCode = CANCEL_NOT_FOUND;
			strDescription = "Payment with given ID does not exist";
		}
	}
};
