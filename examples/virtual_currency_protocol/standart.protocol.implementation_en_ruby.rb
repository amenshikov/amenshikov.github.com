#!/usr/bin/env ruby
# Virtual Currency Protocol Implementation. Ruby Sample.
#
# @version 1.0
# @author Xsolla


require 'digest/md5'
require 'mysql'


# Config class
class XsollaConfig
	# Parameters for database connection
	# @var string
	attr_accessor					:m_dbHost
	attr_accessor					:m_dbUser
	attr_accessor					:m_dbPassword
	attr_accessor					:m_db

	# Table that contains the information about the payments
	# @var string
	attr_accessor					:m_dbPaymentsTable

	# Table where you store character's names or accounts
	# @var string
	attr_accessor					:m_dbCharactersTable

	# Secret key for your project, you can ask for it your account manager
	# @var string
	attr_accessor					:m_SecretKey

	# List of allowed IP
	# @var List<string>
	attr_accessor					:m_AllowedIPs
end


# Class that implements "Virtual Currency Protocol"
#
# In this sample class you can find implementation of main methods of "Virtual Currency Protocol".
# To start script, replace parameters by your own parameters (based on your system).
#
# @version 1.0
# @author Xsolla
class VirtualCurrencyProtocolImplementation
	@m_Config
	@m_Connect

	# Result codes
	module ResultCode
		SUCCESS							= 0
		TEMPORARY_ERROR					= 1
		CANCEL_NOT_FOUND				= 2
		INCORRECT_SIGNATURE				= 3
		INCORRECT_REQUEST_FORMAT		= 4
		OTHER_ERROR						= 5
		PAYMENT_CANNOT_BE_PROCESSED		= 6
	end


	# Instantiates the VirtualCurrencyProtocolImplementation class
	def initialize(oConfig, strRemoteAddress, strCommand, strId, strSum, strV1, strV2, strV3, strMD5)
		@m_Config = oConfig;

		begin
			@m_Connect = Mysql::new(oConfig.m_dbHost, oConfig.m_dbUser, oConfig.m_dbPassword, oConfig.m_db);
			ProcessRequest(strRemoteAddress, strCommand, strId, strSum, strV1, strV2, strV3, strMD5);
		rescue Exception => e
			ErrorCheckResponse("Unable to connect to database: " + e.to_s);
		ensure
			if(@m_Connect)
				@m_Connect.close();
			end
		end
	end

	# Checks signature
	# @param string strSecretKey
	# @param string strMD5
	# @return bool
	def CheckSignature(strSecretKey, strMD5)
		strResult = Digest::MD5.hexdigest(strSecretKey);

		if(strMD5.downcase != strResult.downcase)
			return(false);
		end

		return(true);
	end

	# Method for nickname check
	# @param string strV1
	# @param string strV2
	# @param string strV3
	# @param string strMD5
	def ProcessCheckRequest(strV1, strV2, strV3, strMD5)
		begin
			iCode = ResultCode::OTHER_ERROR;
			strDescription = "";

			# Checking existance of v1 - nickname, account and so on
			if(strV1 == "")
				raise "User ID is undefined"
			end

			# Checking existance of v2, v3. If you don't support these parametres, please comment out these lines
			if(strV2 == "")
				raise "User ID is undefined"
			end

			if(strV3 == "")
				raise "User ID is undefined"
			end

			# Checking existance of md5 param
			if(strMD5 == "")
				raise "Signature is undefined"
			end

			# Checking signature
			if(CheckSignature("check" + strV1 + @m_Config.m_SecretKey, strMD5) == false)
				raise "Incorrect signature"
			end

			iCode = Check(strV1, strV2, strV3, strDescription);

			# Generating response
			GenerateCheckResponse(iCode, strDescription);
		rescue Exception => strMsg
			ErrorCheckResponse(strMsg.to_s);
		end
	end

	# Pay method
	# @param string strId
	# @param string strSum
	# @param string strV1
	# @param string strV2
	# @param string strV3
	# @param string strMD5
	def ProcessPayRequest(strId, strSum, strV1, strV2, strV3, strMD5)
		begin
			iCode = ResultCode::OTHER_ERROR;
			strDescription = "";
			strIdShop = "";

			# Neccessary checking
			if(strId == "")
				raise "Invoice is undefined"
			end

			if(strV1 == "")
				raise "User ID is undefined"
			end

			# If you don't support these parameters, please comment out these lines
			if(strV2 == "")
				raise "User ID is undefined"
			end

			if(strV3 == "")
				raise "User ID is undefined"
			end

			if(strSum == "")
				raise "Amount is undefined"
			end

			if(strMD5 == "")
				raise "Signature is undefined"
			end

			if(CheckSignature("pay" + strV1 + strId + @m_Config.m_SecretKey, strMD5) == false)
				raise "Incorrect signature"
			end

			iCode = Pay(strId, strSum, strV1, strV2, strV3, strDescription, strIdShop);
			GeneratePayResponse(iCode, strDescription, strId, strIdShop, strSum);
		rescue Exception => strMsg
			ErrorPayResponse(strMsg.to_s);
		end
	end

	# Payment cancel method
	# @param string strId
	# @param string strMD5
	def ProcessCancelRequest(strId, strMD5)
		begin
			iCode = ResultCode::OTHER_ERROR;
			strDescription = "";

			if (strId == "")
				raise "Invoice is undefined"
			end

			if(strMD5 == "")
				raise "Signature is undefined"
			end

			if(CheckSignature("cancel" + strId + @m_Config.m_SecretKey, strMD5) == false)
				raise "Incorrect signature"
			end

			iCode = Cancel(strId, strDescription);

			# Generate response
			GenerateCancelResponse(iCode, strDescription);
		rescue Exception => strMsg
			ErrorCancelResponse(strMsg.to_s);
		end
	end

	# Main method
	# @param string strRemoteAddress
	# @param string strCommand
	# @param string strId
	# @param string strSum
	# @param string strV1
	# @param string strV2
	# @param string strV3
	# @param string strMD5
	def ProcessRequest(strRemoteAddress, strCommand, strId, strSum, strV1, strV2, strV3, strMD5)
		begin
			# Checking the IP-address
			if(@m_Config.m_AllowedIPs.include?(strRemoteAddress) == false)
				raise "IP address is not allowed"
			end

			if(strCommand == "")
				raise "Command is undefined"
			end

			if(strCommand == "check")
				ProcessCheckRequest(strV1, strV2, strV3, strMD5);
			elsif(strCommand == "pay")
				ProcessPayRequest(strId, strSum, strV1, strV2, strV3, strMD5);
			elsif(strCommand == "cancel")
				ProcessCancelRequest(strId, strMD5);
			else
				raise "Incorrect command"
			end
		rescue Exception => strMsg
			ErrorCheckResponse(strMsg.to_s);
		end
	end

	# Generating response when using nickname check method
	# @param int iCode
	# @param string strDescription
	def GenerateCheckResponse(iCode, strDescription)
		strXML = "<?xml version=\"1.0\"?>";
		strXML += "<response>";
		strXML += "<result>" + iCode.to_s + "</result>";
		strXML += "<comment>" + strDescription + "</comment>";
		strXML += "</response>";

		print "Content-Type", "text/xml; charset=utf-8\r\n\r\n";
		print strXML;
	end

	# Generating response when using pay method
	# @param int iCode
	# @param string strDescription
	# @param string strInvoice
	# @param string strOrder
	# @param string strSum
	def GeneratePayResponse(iCode, strDescription, strInvoice, strOrder, strSum)
		strXML = "<?xml version=\"1.0\"?>";
		strXML += "<response>";
		strXML += "<id>" + strInvoice + "</id>";
		strXML += "<id_shop>" + strOrder + "</id_shop>";
		strXML += "<sum>" + strSum + "</sum>";
		strXML += "<result>" + iCode.to_s + "</result>";
		strXML += "<comment>" + strDescription + "</comment>";
		strXML += "</response>";

		print "Content-Type", "text/xml; charset=utf-8\r\n\r\n";
		print strXML;
	end

	# Generating response when using payment cancel method
	# @param int iCode
	# @param string strDescription
	def GenerateCancelResponse(iCode, strDescription)
		strXML = "<?xml version=\"1.0\"?>";
		strXML += "<response>";
		strXML += "<result>" + iCode.to_s + "</result>";
		strXML += "<comment>" + strDescription + "</comment>";
		strXML += "</response>";

		print "Content-Type", "text/xml; charset=utf-8\r\n\r\n";
		print strXML;
	end

	def ErrorCheckResponse(strMessage)
		GenerateCheckResponse(ResultCode::PAYMENT_CANNOT_BE_PROCESSED, strMessage);
	end

	def ErrorPayResponse(strMessage)
		GeneratePayResponse(ResultCode::PAYMENT_CANNOT_BE_PROCESSED, strMessage, "", "", "");
	end

	def ErrorCancelResponse(strMessage)
		GenerateCancelResponse(ResultCode::PAYMENT_CANNOT_BE_PROCESSED, strMessage);
	end

	#####################################
	# THIS SECTION IS THE ONLY PLACE WHERE YOU SHOULD WRITE CODE
	#####################################

	# YOUR CHECK CODE HERE
	def Check(strV1, strV2, strV3, strDescription)
		# Code example

		# If you don't use v2, v3, use the below
		#strCommand = "SELECT count(1) as cnt FROM " + @m_Config.m_dbCharactersTable + " WHERE v1 = ?";
		strCommand = "SELECT count(1) as cnt FROM " + @m_Config.m_dbCharactersTable + " WHERE v1 = ? AND v2 = ? AND v3 = ?";
		oCommand = @m_Connect.prepare(strCommand);
		# performing query
		oResult = oCommand.execute(strV1, strV2, strV3);
		# if nickname exists
		oRow = oResult.fetch();
		if(oRow[0] > 0)
			iCode = ResultCode::SUCCESS;
			strDescription.replace("OK");
		else
			iCode = ResultCode::PAYMENT_CANNOT_BE_PROCESSED;
			strDescription.replace("Character doesn't exist.");
		end
		return(iCode);
	end

	# YOUR PAY CODE HERE
	def Pay(strId, strSum, strV1, strV2, strV3, strDescription, strIdShop)
		# Code example

		# Looking for payment with such id
		strCommand = "SELECT `id` FROM " + @m_Config.m_dbPaymentsTable + " WHERE `invoice` = ?";
		oCommand = @m_Connect.prepare(strCommand);
		# performing query
		oResult = oCommand.execute(strId);
		# getting result
		oRow = oCommand.fetch();
		# If there is no payment with this id, insert it
		if(!oRow)
			# If you don't use v2, v3, use the below
			#strCommand = "INSERT INTO `" + @m_Config.m_dbPaymentsTable + "` (`v1`, `amount`, `invoice`, `date_add`, `canceled`) VALUES (?, ?, ?, NOW(), \"0\")";
			strCommand = "INSERT INTO `" + @m_Config.m_dbPaymentsTable + "` (`v1`, `v2`, `v3`, `amount`, `invoice`, `date_add`, `canceled`) VALUES (?, ?, ?, ?, ?, NOW(), \"0\")";
			oCommand = @m_Connect.prepare(strCommand);
			# performing query
			oResult = oCommand.execute(strV1, strV2, strV3, strSum, strId);
			# if insert successful
			strIdShop.replace("");
			if(oResult.affected_rows() == 1)
				strCommand = "SELECT LAST_INSERT_ID()";
				oCommand = @m_Connect.prepare(strCommand);
				oResult = oCommand.execute();
				oRow = oResult.fetch();
				strIdShop.replace(oRow[0].to_s);
			end
			if(Integer(strIdShop) > 0)
				iCode = ResultCode::SUCCESS;
				strDescription.replace("OK");
			else
				iCode = ResultCode::OTHER_ERROR;
				strDescription.replace("Couldn't insert data. See mysql errors");
			end
		else
			# If payment with such xsolla id exists, successful
			iCode = ResultCode::SUCCESS;
			strDescription.replace("OK");
			strIdShop.replace(oRow[0].to_s);
		end
		return(iCode);
	end

	# YOUR CANCEL CODE HERE
	def Cancel(strId, strDescription)
		# Code example

		# Canceling payment
		strCommand = "UPDATE `" + @m_Config.m_dbPaymentsTable + "` SET `canceled` = \"1\", `date_cancel` = NOW() WHERE `invoice` = ?";
		oCommand = @m_Connect.prepare(strCommand);
		# performing query
		oResult = oCommand.execute(strId);
		if(oResult.affected_rows() > 0)
			iCode = ResultCode::SUCCESS;
			strDescription.replace("OK");
		else
			iCode = ResultCode::CANCEL_NOT_FOUND;
			strDescription.replace("Payment with given ID does not exist");
		end
		return(iCode);
	end
end
