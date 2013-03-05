// Virtual Currency Protocol Implementation. ASP.NET/C# Sample.
//
// @version 1.0
// @author Xsolla


using System;
using System.Collections.Generic;
using System.Text;
using MySql.Data.MySqlClient;


// Config class
class XsollaConfig
{
	// Parameters for database connection
	// @var string
	public string					m_dbHost;
	public string					m_dbUser;
	public string					m_dbPassword;
	public string					m_db;

	// Table that contains the information about the payments
	// @var string
	public string					m_dbPaymentsTable;

	// Table where you store character's names or accounts
	// @var string
	public string					m_dbCharactersTable;

	// Secret key for your project, you can ask for it your account manager
	// @var string
	public string					m_SecretKey;

	// List of allowed IP
	// @var List<string>
	public List<string>				m_AllowedIPs;
};


// Class that implements "Virtual Currency Protocol"
//
// In this sample class you can find implementation of main methods of "Virtual Currency Protocol".
// To start script, replace parameters by your own parameters (based on your system).
//
// @version 1.0
// @author Xsolla
class VirtualCurrencyProtocolImplementation1
{
	private XsollaConfig			m_Config;
	private MySqlConnection			m_Connect;

	// Result codes
	private enum ResultCode
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
	public VirtualCurrencyProtocolImplementation1(XsollaConfig Config, string strRemoteAddress, string strCommand, string strId, string strSum, string strV1, string strV2, string strV3, string strMD5)
	{
		string							strConnectionString;

		m_Config = Config;
		m_Connect = null;

		try
		{
			strConnectionString = "server=" + Config.m_dbHost + ";userid=" + Config.m_dbUser + ";password=" + Config.m_dbPassword + ";database=" + Config.m_db;
			m_Connect = new MySqlConnection(strConnectionString);
			m_Connect.Open();
			ProcessRequest(strRemoteAddress, strCommand, strId, strSum, strV1, strV2, strV3, strMD5);
		}
		catch(Exception e)
		{
			ErrorCheckResponse("Unable to connect to database: " + e.ToString());
		}
	}

	~VirtualCurrencyProtocolImplementation1()
	{
		if(m_Connect != null)
			m_Connect.Close();
		m_Connect = null;
	}

	// Checks signature
	// @param string strSecretKey
	// @param string strMD5
	// @return bool
	private bool CheckSignature(string strSecretKey, string strMD5)
	{
		MD5								md5Hash;
		byte []							pResult;
		StringBuilder					oStringBuilder;
		string							strResult;

		md5Hash = MD5.Create();
		pResult = md5Hash.ComputeHash(Encoding.UTF8.GetBytes(strSecretKey));

		oStringBuilder = new StringBuilder();
		for(int i=0; i<pResult.Length; i++)
			oStringBuilder.Append(pResult[i].ToString("x2"));

		strResult = oStringBuilder.ToString();

		if(strMD5.ToLower().CompareTo(strResult) != 0)
			return(false);

		return(true);
	}

	// Method for nickname check
	// @param string strV1
	// @param string strV2
	// @param string strV3
	// @param string strMD5
	private void ProcessCheckRequest(string strV1, string strV2, string strV3, string strMD5)
	{
		try
		{
			ResultCode						iCode = ResultCode.OTHER_ERROR;
			string							strDescription = "";

			// Checking existance of v1 - nickname, account and so on
			if(strV1.Length <= 0)
				throw new ArgumentException("User ID is undefined");

			// Checking existance of v2, v3. If you don't support these parametres, please comment out these lines
			if(strV2.Length <= 0)
				throw new ArgumentException("User ID is undefined");

			if(strV3.Length <= 0)
				throw new ArgumentException("User ID is undefined");

			// Checking existance of md5 param
			if(strMD5.Length <= 0)
				throw new ArgumentException("Signature is undefined");

			// Checking signature
			if(CheckSignature("check" + strV1 + m_Config.m_SecretKey, strMD5) == false)
				throw new Exception("Incorrect signature");

			Check(strV1, strV2, strV3, ref iCode, ref strDescription);

			// Generating response
			GenerateCheckResponse(iCode, strDescription);
		}
		catch(Exception strMsg)
		{
			ErrorCheckResponse(strMsg.ToString());
		}
	}

	// Pay method
	// @param string strId
	// @param string strSum
	// @param string strV1
	// @param string strV2
	// @param string strV3
	// @param string strMD5
	private void ProcessPayRequest(string strId, string strSum, string strV1, string strV2, string strV3, string strMD5)
	{
		try
		{
			ResultCode						iCode = ResultCode.OTHER_ERROR;
			string							strDescription = "";
			string							strIdShop = "";

			// Neccessary checking
			if(strId.Length <= 0)
				throw new ArgumentException("Invoice is undefined");

			if(strV1.Length <= 0)
				throw new ArgumentException("User ID is undefined");

			// If you don't support these parameters, please comment out these lines
			if(strV2.Length <= 0)
				throw new ArgumentException("User ID is undefined");

			if(strV3.Length <= 0)
				throw new ArgumentException("User ID is undefined");

			if(strSum.Length <= 0)
				throw new ArgumentException("Amount is undefined");

			if(strMD5.Length <= 0)
				throw new ArgumentException("Signature is undefined");

			if(CheckSignature("pay" + strV1 + strId + m_Config.m_SecretKey, strMD5) == false)
				throw new Exception("Incorrect signature");

			Pay(strId, strSum, strV1, strV2, strV3, ref iCode, ref strDescription, ref strIdShop);
			GeneratePayResponse(iCode, strDescription, strId, strIdShop, strSum);
		}
		catch(Exception strMsg)
		{
			ErrorPayResponse(strMsg.ToString());
		}
	}

	// Payment cancel method
	// @param string strId
	// @param string strMD5
	private void ProcessCancelRequest(string strId, string strMD5)
	{
		try
		{
			ResultCode						iCode = ResultCode.OTHER_ERROR;
			string							strDescription = "";

			if (strId.Length <= 0)
				throw new ArgumentException("Invoice is undefined");

			if(strMD5.Length <= 0)
				throw new ArgumentException("Signature is undefined");

			if(CheckSignature("cancel" + strId + m_Config.m_SecretKey, strMD5) == false)
				throw new Exception("Incorrect signature");

			Cancel(strId, ref iCode, ref strDescription);

			// Generate response
			GenerateCancelResponse(iCode, strDescription);
		}
		catch(Exception strMsg)
		{
			ErrorCancelResponse(strMsg.ToString());
		}
	}

	// Main method
	// @param string strRemoteAddress
	// @param string strCommand
	// @param string strId
	// @param string strSum
	// @param string strV1
	// @param string strV2
	// @param string strV3
	// @param string strMD5
	private void ProcessRequest(string strRemoteAddress, string strCommand, string strId, string strSum, string strV1, string strV2, string strV3, string strMD5)
	{
		try
		{
			// Checking the IP-address
			if(m_Config.m_AllowedIPs.Contains(strRemoteAddress) == false)
				throw new ArgumentException("IP address is not allowed");

			if(strCommand.Length <= 0)
				throw new ArgumentException("Command is undefined");

			if(strCommand.CompareTo("check") == 0)
				ProcessCheckRequest(strV1, strV2, strV3, strMD5);
			else if(strCommand.CompareTo("pay") == 0)
				ProcessPayRequest(strId, strSum, strV1, strV2, strV3, strMD5);
			else if(strCommand.CompareTo("cancel") == 0)
				ProcessCancelRequest(strId, strMD5);
			else
				throw new ArgumentException("Incorrect command");
		}
		catch(Exception strMsg)
		{
			ErrorCheckResponse(strMsg.ToString());
		}
	}

	// Generating response when using nickname check method
	// @param ResultCode iCode
	// @param string strDescription
	private void GenerateCheckResponse(ResultCode iCode, string strDescription)
	{
		string							strXML;

		strXML = "<?xml version=\"1.0\"?>";
		strXML += "<response>";
		strXML += "<result>" + iCode + "</result>";
		strXML += "<comment>" + strDescription + "</comment>";
		strXML += "</response>";

//			Response.Headers.Set("Content-Type", "text/xml; charset=utf-8");
//			Response.Write(strXML);
		Console.WriteLine(strXML);
	}

	// Generating response when using pay method
	// @param ResultCode iCode
	// @param string strDescription
	// @param string strInvoice
	// @param string strOrder
	// @param string strSum
	private void GeneratePayResponse(ResultCode iCode, string strDescription, string strInvoice, string strOrder, string strSum)
	{
		string							strXML;

		strXML = "<?xml version=\"1.0\"?>";
		strXML += "<response>";
		strXML += "<id>" + strInvoice + "</id>";
		strXML += "<id_shop>" + strOrder + "</id_shop>";
		strXML += "<sum>" + strSum + "</sum>";
		strXML += "<result>" + iCode + "</result>";
		strXML += "<comment>" + strDescription + "</comment>";
		strXML += "</response>";

//			Response.Headers.Set("Content-Type", "text/xml; charset=utf-8");
//			Response.Write(strXML);
		Console.WriteLine(strXML);
	}

	// Generating response when using payment cancel method
	// @param int iCode
	// @param string strDescription
	private void GenerateCancelResponse(ResultCode iCode, string strDescription)
	{
		string							strXML;

		strXML = "<?xml version=\"1.0\"?>";
		strXML += "<response>";
		strXML += "<result>" + iCode + "</result>";
		strXML += "<comment>" + strDescription + "</comment>";
		strXML += "</response>";

//			Response.Headers.Set("Content-Type", "text/xml; charset=utf-8");
//			Response.Write(strXML);
		Console.WriteLine(strXML);
	}


	private void ErrorCheckResponse(string strMessage)
	{
		GenerateCheckResponse(ResultCode.PAYMENT_CANNOT_BE_PROCESSED, strMessage);
	}

	private void ErrorPayResponse(string strMessage)
	{
		GeneratePayResponse(ResultCode.PAYMENT_CANNOT_BE_PROCESSED, strMessage, "", "", "");
	}

	private void ErrorCancelResponse(string strMessage)
	{
		GenerateCancelResponse(ResultCode.PAYMENT_CANNOT_BE_PROCESSED, strMessage);
	}

	//////////////////////////////////////////////////////////////////////////
	////// THIS SECTION IS THE ONLY PLACE WHERE YOU SHOULD WRITE CODE ////////
	//////////////////////////////////////////////////////////////////////////

	// YOUR CHECK CODE HERE
	private void Check(string strV1, string strV2, string strV3, ref ResultCode iCode, ref string strDescription)
	{
		MySqlCommand					oCommand;
		Int64							iCount;

		// Code example

		// If you don't use v2, v3, use the string below
		//oCommand = new MySqlCommand("SELECT count(1) as cnt FROM " + m_Config.m_dbCharactersTable + " WHERE v1 = @V1", m_Connect);
		oCommand = new MySqlCommand("SELECT count(1) as cnt FROM " + m_Config.m_dbCharactersTable + " WHERE v1 = @V1 AND v2 = @V2 AND v3 = @V3", m_Connect);
		oCommand.Prepare();
		oCommand.Parameters.AddWithValue("@V1", strV1);
		oCommand.Parameters.AddWithValue("@V2", strV2);
		oCommand.Parameters.AddWithValue("@V3", strV3);
		// performing query
		iCount = Convert.ToInt64(oCommand.ExecuteScalar());
		// if nickname exists
		if(iCount > 0)
		{
			iCode = ResultCode.SUCCESS;
			strDescription = "OK";
		}
		else
		{
			iCode = ResultCode.PAYMENT_CANNOT_BE_PROCESSED;
			strDescription = "Character doesn't exist.";
		}
	}

	// YOUR PAY CODE HERE
	private void Pay(string strId, string strSum, string strV1, string strV2, string strV3, ref ResultCode iCode, ref string strDescription, ref string strIdShop)
	{
		MySqlCommand					oCommand;
		UInt32							iId;

		// Code example

		// Looking for payment with such id
		oCommand = new MySqlCommand("SELECT `id` FROM " + m_Config.m_dbPaymentsTable + " WHERE `invoice` = @Id", m_Connect);
		oCommand.Prepare();
		oCommand.Parameters.AddWithValue("@Id", strId);
		// performing query
		iId = Convert.ToUInt32(oCommand.ExecuteScalar());
		// getting result
		// If there is no payment with this id, insert it
		if(iId == 0)
		{
			// If you don't use v2, v3, use the string below
			//strSQL = string("INSERT INTO `") + m_Config.m_dbPaymentsTable + string("` (`v1`, `amount`, `invoice`, `date_add`, `canceled`) VALUES (") + strV1 + string(", ") + strSum + string(", ") + strId + string(", NOW(), \"0\")");
			//oCommand = new MySqlCommand("INSERT INTO `" + m_Config.m_dbPaymentsTable + "` (`v1`, `amount`, `invoice`, `date_add`, `canceled`) VALUES (@V1, @Sum, @Id, NOW(), \"0\")", m_Connect);
			oCommand = new MySqlCommand("INSERT INTO `" + m_Config.m_dbPaymentsTable + "` (`v1`, `v2`, `v3`, `amount`, `invoice`, `date_add`, `canceled`) VALUES (@V1, @V2, @V3, @Sum, @Id, NOW(), \"0\")", m_Connect);
			oCommand.Prepare();
			oCommand.Parameters.AddWithValue("@V1", strV1);
			oCommand.Parameters.AddWithValue("@V2", strV2);
			oCommand.Parameters.AddWithValue("@V3", strV3);
			oCommand.Parameters.AddWithValue("@Sum", strSum);
			oCommand.Parameters.AddWithValue("@Id", strId);
			// performing query
			if(oCommand.ExecuteNonQuery() != 1)
				throw new Exception("Failed to query database");
			// if insert successful
			oCommand = new MySqlCommand("SELECT LAST_INSERT_ID()", m_Connect);
			oCommand.Prepare();
			strIdShop = Convert.ToString(oCommand.ExecuteScalar());
			if((strIdShop.Length > 0) && (strIdShop.CompareTo("0") != 0))
			{
				iCode = ResultCode.SUCCESS;
				strDescription = "OK";
			}
			else
			{
				iCode = ResultCode.OTHER_ERROR;
				strDescription = "Couldn't insert data. See mysql errors";
			}
		}
		else
		{
			// If payment with such xsolla id exists, successful
			iCode = ResultCode.SUCCESS;
			strDescription = "OK";
			strIdShop = iId.ToString();
		}
	}

	// YOUR CANCEL CODE HERE
	private void Cancel(string strId, ref ResultCode iCode, ref string strDescription)
	{
		MySqlCommand					oCommand;

		// Code example

		// Canceling payment
		oCommand = new MySqlCommand("UPDATE `" + m_Config.m_dbPaymentsTable + "` SET `canceled` = \"1\", `date_cancel` = NOW() WHERE `invoice` = @Id", m_Connect);
		oCommand.Prepare();
		oCommand.Parameters.AddWithValue("@Id", strId);
		// performing query
		if(oCommand.ExecuteNonQuery() > 0)
		{
			iCode = ResultCode.SUCCESS;
			strDescription = "OK";
		}
		else
		{
			iCode = ResultCode.CANCEL_NOT_FOUND;
			strDescription = "Payment with given ID does not exist";
		}
	}
};
