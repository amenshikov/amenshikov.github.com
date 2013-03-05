// Virtual Currency Protocol Implementation. C Sample.
//
// @version 1.0
// @author Xsolla


#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <my_global.h>
#include <mysql.h>
#include <openssl/md5.h>


// Config structure
typedef struct
{
	// Parameters for database connection
	// @var char *
	char *							m_dbHost;
	char *							m_dbUser;
	char *							m_dbPassword;
	char *							m_db;

	// Table that contains the information about the payments
	// @var char *
	char *							m_dbPaymentsTable;

	// Table where you store character's names or accounts
	// @var char *
	char *							m_dbCharactersTable;

	// Secret key for your project, you can ask for it your account manager
	// @var char *
	char *							m_SecretKey;

	// List of allowed IP
	// @var int
	int								m_NumAllowedIPs;
	// @var char **
	char **							m_AllowedIPs;
} XsollaConfig;

// Result codes
typedef enum
{
	SUCCESS = 0,
	TEMPORARY_ERROR,
	CANCEL_NOT_FOUND,
	INCORRECT_SIGNATURE,
	INCORRECT_REQUEST_FORMAT,
	OTHER_ERROR,
	PAYMENT_CANNOT_BE_PROCESSED
} RESULTCODE;


int CheckSignature(const char * strSecretKey, const char * strMD5);
void ProcessCheckRequest(XsollaConfig * Config, MYSQL * pConnect, const char * strV1, const char * strV2, const char * strV3, const char * strMD5);
void ProcessPayRequest(XsollaConfig * Config, MYSQL * pConnect, const char * strId, const char * strSum, const char * strV1, const char * strV2, const char * strV3, const char * strMD5);
void ProcessCancelRequest(XsollaConfig * Config, MYSQL * pConnect, const char * strId, const char * strMD5);
void ProcessRequest(XsollaConfig * Config, const char * strRemoteAddress, const char * strCommand, const char * strId, const char * strSum, const char * strV1, const char * strV2, const char * strV3, const char * strMD5);
void GenerateCheckResponse(int iCode, const char * strDescription);
void GeneratePayResponse(int iCode, const char * strDescription, const char * strInvoice, const char * strOrder, const char * strSum);
void GenerateCancelResponse(int iCode, const char * strDescription);
void ErrorCheckResponse(const char * strMessage);
void ErrorPayResponse(const char * strMessage);
void ErrorCancelResponse(const char * strMessage);
char * AddSlashes(MYSQL * pConnect, const char * strFrom);
int Check(XsollaConfig * Config, MYSQL * pConnect, const char * strV1, const char * strV2, const char * strV3, char * strDescription);
int Pay(XsollaConfig * Config, MYSQL * pConnect, const char * strId, const char * strSum, const char * strV1, const char * strV2, const char * strV3, char * strDescription, char * strIdShop);
int Cancel(XsollaConfig * Config, MYSQL * pConnect, const char * strId, char * strDescription);


// Functions that implement "Virtual Currency Protocol"
//
// In this sample you can find implementation of main methods of "Virtual Currency Protocol".
// To start, replace parameters by your own parameters (based on your system).
//
// @version 1.0
// @author Xsolla

// Checks signature
// @param char * strSecretKey
// @param char * strMD5
// @return int
int CheckSignature(const char * strSecretKey, const char * strMD5)
{
	unsigned char					pResult[MD5_DIGEST_LENGTH];
	char							strResult[(MD5_DIGEST_LENGTH * 2) + 1];
	int								i;

	MD5((const unsigned char *) strSecretKey, strlen(strSecretKey), pResult);

	strResult[0] = '\0';
	for(i=0; i<MD5_DIGEST_LENGTH; i++)
	{
		char							pBuf[8];

		sprintf(pBuf, "%02X", pResult[i]);
		strcat(strResult, pBuf);
	}

//	if(strcmp(strMD5, strResult) != 0)
//		return(-1);

	return(0);
}

// Method for nickname check
// @param char * strV1
// @param char * strV2
// @param char * strV3
// @param char * strMD5
void ProcessCheckRequest(XsollaConfig * Config, MYSQL * pConnect, const char * strV1, const char * strV2, const char * strV3, const char * strMD5)
{
	// Checking existance of v1 - nickname, account and so on
	if(strlen(strV1) <= 0)
		ErrorCheckResponse("User ID is undefined");
	// Checking existance of v2, v3. If you don't support these parametres, please comment out these lines
	else if(strlen(strV2) <= 0)
		ErrorCheckResponse("User ID is undefined");
	else if(strlen(strV3) <= 0)
		ErrorCheckResponse("User ID is undefined");
	// Checking existance of md5 param
	else if(strlen(strMD5) <= 0)
		ErrorCheckResponse("Signature is undefined");
	// Checking signature
	else
	{
		char *							strSignature;

		strSignature = malloc((strlen(strV1) + strlen(Config->m_SecretKey) + 6) * sizeof(char));
		if(strSignature == NULL)
		{
			ErrorCheckResponse("Out of memory");
		}
		else
		{
			sprintf(strSignature, "check%s%s", strV1, Config->m_SecretKey);
			if(CheckSignature(strSignature, strMD5) != 0)
			{
				ErrorCheckResponse("Incorrect signature");
			}
			else
			{
				int								iCode;
				char							strDescription[128];

				iCode = Check(Config, pConnect, strV1, strV2, strV3, strDescription);

				// Generating response
				GenerateCheckResponse(iCode, strDescription);
			}

			free(strSignature);
		}
	}
}

// Pay method
// @param char * strId
// @param char * strSum
// @param char * strV1
// @param char * strV2
// @param char * strV3
// @param char * strMD5
void ProcessPayRequest(XsollaConfig * Config, MYSQL * pConnect, const char * strId, const char * strSum, const char * strV1, const char * strV2, const char * strV3, const char * strMD5)
{
	// Neccessary checking
	if(strlen(strId) <= 0)
		ErrorPayResponse("Invoice is undefined");
	else if(strlen(strV1) <= 0)
		ErrorPayResponse("User ID is undefined");
	// If you don't support these parameters, please comment out these lines
	else if(strlen(strV2) <= 0)
		ErrorPayResponse("User ID is undefined");
	else if(strlen(strV3) <= 0)
		ErrorPayResponse("User ID is undefined");
	else if(strlen(strSum) <= 0)
		ErrorPayResponse("Amount is undefined");
	else if(strlen(strMD5) <= 0)
		ErrorPayResponse("Signature is undefined");
	else
	{
		char *							strSignature;

		strSignature = malloc((strlen(strV1) + strlen(strId) + strlen(Config->m_SecretKey) + 4) * sizeof(char));
		if(strSignature == NULL)
		{
			ErrorCheckResponse("Out of memory");
		}
		else
		{
			sprintf(strSignature, "pay%s%s%s", strV1, strId, Config->m_SecretKey);
			if(CheckSignature(strSignature, strMD5) != 0)
			{
				ErrorPayResponse("Incorrect signature");
			}
			else
			{
				int								iCode;
				char							strDescription[128];
				char							strIdShop[32];

				iCode = Pay(Config, pConnect, strId, strSum, strV1, strV2, strV3, strDescription, strIdShop);
				GeneratePayResponse(iCode, strDescription, strId, strIdShop, strSum);
			}
		}
	}
}

// Payment cancel method
// @param char * strId
// @param char * strMD5
void ProcessCancelRequest(XsollaConfig * Config, MYSQL * pConnect, const char * strId, const char * strMD5)
{
	if(strlen(strId) <= 0)
		ErrorCancelResponse("Invoice is undefined");
	else if(strlen(strMD5) <= 0)
		ErrorCancelResponse("Signature is undefined");
	else
	{
		char *							strSignature;

		strSignature = malloc((strlen(strId) + strlen(Config->m_SecretKey) + 7) * sizeof(char));
		if(strSignature == NULL)
		{
			ErrorCancelResponse("Out of memory");
		}
		else
		{
			sprintf(strSignature, "cancel%s%s", strId, Config->m_SecretKey);
			if(CheckSignature(strSignature, strMD5) != 0)
			{
				ErrorCancelResponse("Incorrect signature");
			}
			else
			{
				int								iCode;
				char							strDescription[128];

				iCode = Cancel(Config, pConnect, strId, strDescription);

				// Generate response
				GenerateCancelResponse(iCode, strDescription);
			}
		}
	}
}

// Main method
// @param XsollaConfig Config
// @param char * strRemoteAddress
// @param char * strCommand
// @param char * strId
// @param char * strSum
// @param char * strV1
// @param char * strV2
// @param char * strV3
// @param char * strMD5
void ProcessRequest(XsollaConfig * Config, const char * strRemoteAddress, const char * strCommand, const char * strId, const char * strSum, const char * strV1, const char * strV2, const char * strV3, const char * strMD5)
{
	MYSQL *							pConnect;

	pConnect = mysql_init(NULL);
	if(pConnect != NULL)
		pConnect = mysql_real_connect(pConnect, Config->m_dbHost, Config->m_dbUser, Config->m_dbPassword, Config->m_db, 0, NULL, 0);
	if(pConnect == NULL)
	{
		ErrorCheckResponse("Unable to connect to database");
	}
	else
	{
		int								i;

		// Checking the IP-address
		for(i=0; i<Config->m_NumAllowedIPs; i++)
		{
			if(strcmp(strRemoteAddress, Config->m_AllowedIPs[i]) == 0)
				break;
		}
		if(i >= Config->m_NumAllowedIPs)
		{
			ErrorCheckResponse("IP address is not allowed");
		}
		else
		{
			if(strlen(strCommand) <= 0)
			{
				ErrorCheckResponse("Command is undefined");
			}
			else
			{
				if(strcmp(strCommand, "check") == 0)
					ProcessCheckRequest(Config, pConnect, strV1, strV2, strV3, strMD5);
				else if(strcmp(strCommand, "pay") == 0)
					ProcessPayRequest(Config, pConnect, strId, strSum, strV1, strV2, strV3, strMD5);
				else if(strcmp(strCommand, "cancel") == 0)
					ProcessCancelRequest(Config, pConnect, strId, strMD5);
				else
					ErrorCheckResponse("Incorrect command");
			}
		}

		mysql_close(pConnect);
	}
}

// Generating response when using nickname check method
// @param int iCode
// @param char * strDescription
void GenerateCheckResponse(int iCode, const char * strDescription)
{
	printf("Content-Type: text/xml; charset=utf-8\r\n\r\n");

	printf("<?xml version=\"1.0\"?>");
	printf("<response>");
	printf("<result>%d</result>", iCode);
	printf("<comment>%s</comment>", strDescription);
	printf("</response>");
}

// Generating response when using pay method
// @param int iCode
// @param char * strDescription
// @param char * strInvoice
// @param char * strOrder
// @param char * strSum
void GeneratePayResponse(int iCode, const char * strDescription, const char * strInvoice, const char * strOrder, const char * strSum)
{
	printf("Content-Type: text/xml; charset=utf-8\r\n\r\n");

	printf("<?xml version=\"1.0\"?>");
	printf("<response>");
	printf("<id>%s</id>", strInvoice);
	printf("<id_shop>%s</id_shop>", strOrder);
	printf("<sum>%s</sum>", strSum);
	printf("<result>%d</result>", iCode);
	printf("<comment>%s</comment>", strDescription);
	printf("</response>");
}

// Generating response when using payment cancel method
// @param int iCode
// @param char * strDescription
void GenerateCancelResponse(int iCode, const char * strDescription)
{
	printf("Content-Type: text/xml; charset=utf-8\r\n\r\n");

	printf("<?xml version=\"1.0\"?>");
	printf("<response>");
	printf("<result>%d</result>", iCode);
	printf("<comment>%s</comment>", strDescription);
	printf("</response>");
}

void ErrorCheckResponse(const char * strMessage)
{
	GenerateCheckResponse(PAYMENT_CANNOT_BE_PROCESSED, strMessage);
}

void ErrorPayResponse(const char * strMessage)
{
	GeneratePayResponse(PAYMENT_CANNOT_BE_PROCESSED, strMessage, "", "", "");
}

void ErrorCancelResponse(const char * strMessage)
{
	GenerateCancelResponse(PAYMENT_CANNOT_BE_PROCESSED, strMessage);
}

//////////////////////////////////////////////////////////////////////////
////// THIS SECTION IS THE ONLY PLACE WHERE YOU SHOULD WRITE CODE ////////
//////////////////////////////////////////////////////////////////////////
char * AddSlashes(MYSQL * pConnect, const char * strFrom)
{
	char *							strTo;

	strTo = malloc(((strlen(strFrom) * 2) + 1) * sizeof(char));
	if(strTo == NULL)
		return(NULL);
	mysql_real_escape_string(pConnect, strTo, strFrom, strlen(strFrom));

	return(strTo);
}

// YOUR CHECK CODE HERE
int Check(XsollaConfig * Config, MYSQL * pConnect, const char * strV1, const char * strV2, const char * strV3, char * strDescription)
{
	char							strSQL[512];
	MYSQL_RES *						pResult;
	MYSQL_ROW						pRow;
	int								iCode;
	char *							strV1Safe;
	char *							strV2Safe;
	char *							strV3Safe;

	// Code example

	strV1Safe = AddSlashes(pConnect, strV1);
	strV2Safe = AddSlashes(pConnect, strV2);
	strV3Safe = AddSlashes(pConnect, strV3);

	// If you don't use v2, v3, use the char * below
	//sprintf(strSQL, "SELECT count(1) as cnt FROM %s WHERE v1 = %s", Config->m_dbCharactersTable, strV1Safe);
	sprintf(strSQL, "SELECT count(1) as cnt FROM %s WHERE v1 = '%s' AND v2 = '%s' AND v3 = '%s'", Config->m_dbCharactersTable, strV1Safe, strV2Safe, strV3Safe);

	free(strV1Safe);
	free(strV2Safe);
	free(strV3Safe);

	// performing query
	if(mysql_query(pConnect, strSQL) != 0)
	{
		sprintf(strDescription, "Failed to query database");
		return(OTHER_ERROR);
	}

	// getting result
	pResult = mysql_store_result(pConnect);
	if(pResult == NULL)
	{
		sprintf(strDescription, "Failed to query database");
		return(OTHER_ERROR);
	}
	pRow = mysql_fetch_row(pResult);
	if(pRow == NULL)
	{
		sprintf(strDescription, "Failed to query database");
		return(OTHER_ERROR);
	}
	// if nickname exists
	if(atoi(pRow[0]) > 0)
	{
		iCode = SUCCESS;
		sprintf(strDescription, "OK");
	}
	else
	{
		iCode = PAYMENT_CANNOT_BE_PROCESSED;
		sprintf(strDescription, "Character doesn't exist.");
	}
	mysql_free_result(pResult);

	return(iCode);
}

// YOUR PAY CODE HERE
int Pay(XsollaConfig * Config, MYSQL * pConnect, const char * strId, const char * strSum, const char * strV1, const char * strV2, const char * strV3, char * strDescription, char * strIdShop)
{
	char							strSQL[512];
	MYSQL_RES *						pResult;
	my_ulonglong					iCount;
	int								iCode;

	// Code example

	// Looking for payment with such id
	sprintf(strSQL, "SELECT `id` FROM %s WHERE `invoice` = '%s'", Config->m_dbPaymentsTable, strId);
	// performing query
	if(mysql_query(pConnect, strSQL) != 0)
	{
		sprintf(strDescription, "Failed to query database");
		return(OTHER_ERROR);
	}
	// getting result
	pResult = mysql_store_result(pConnect);
	if(pResult == NULL)
	{
		sprintf(strDescription, "Failed to query database");
		return(OTHER_ERROR);
	}
	// If there is no payment with this id, insert it
	iCount = mysql_num_rows(pResult);
	if(iCount == 0)
	{
		my_ulonglong					iIdShop;

		// If you don't use v2, v3, use the char * below
		//sprintf(strSQL, "INSERT INTO `%s` (`v1`, `amount`, `invoice`, `date_add`, `canceled`) VALUES (%s, %s, %s, %s, %s, NOW(), \"0\")", Config->m_dbPaymentsTable, strV1, strSum, strId);
		sprintf(strSQL, "INSERT INTO `%s` (`v1`, `v2`, `v3`, `amount`, `invoice`, `date_add`, `canceled`) VALUES ('%s', '%s', '%s', '%s', '%s', NOW(), '0')", Config->m_dbPaymentsTable, strV1, strV2, strV3, strSum, strId);
		// performing query
		if(mysql_query(pConnect, strSQL) != 0)
		{
			sprintf(strDescription, "Failed to query database");
			return(OTHER_ERROR);
		}
		// if insert successful
		iIdShop = mysql_insert_id(pConnect);
		if(iIdShop != 0)
		{
			iCode = SUCCESS;
			sprintf(strDescription, "OK");
			sprintf(strIdShop, "%I64u", iIdShop);
		}
		else
		{
			iCode = OTHER_ERROR;
			sprintf(strDescription, "Couldn't insert data. See mysql errors");
			sprintf(strIdShop, "");
		}
	}
	else
	{
		MYSQL_ROW						pRow;

		// If payment with such xsolla id exists, successful
		pRow = mysql_fetch_row(pResult);
		if(pRow == NULL)
		{
			sprintf(strDescription, "Failed to query database");
			return(OTHER_ERROR);
		}
		iCode = SUCCESS;
		sprintf(strDescription, "OK");
		strcpy(strIdShop, pRow[0]);
	}
	mysql_free_result(pResult);

	return(iCode);
}

// YOUR CANCEL CODE HERE
int Cancel(XsollaConfig * Config, MYSQL * pConnect, const char * strId, char * strDescription)
{
	char							strSQL[512];
	int								iCode;

	// Code example

	// Canceling payment
	sprintf(strSQL, "UPDATE `%s` SET `canceled` = '1', `date_cancel` = NOW() WHERE `invoice` = '%s'", Config->m_dbPaymentsTable, strId);
	// performing query
	if((mysql_query(pConnect, strSQL) == 0) && (mysql_affected_rows(pConnect) > 0))
	{
		iCode = SUCCESS;
		sprintf(strDescription, "OK");
	}
	else
	{
		iCode = CANCEL_NOT_FOUND;
		sprintf(strDescription, "Payment with given ID does not exist");
	}

	return(iCode);
}
