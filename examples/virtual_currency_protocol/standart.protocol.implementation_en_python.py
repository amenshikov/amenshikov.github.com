# Virtual Currency Protocol Implementation. Python Sample.
#
# @version 1.0
# @author Xsolla


import sys
import MySQLdb
import md5


# Config class
class XsollaConfig:
	# Parameters for database connection
	# @var string
	m_dbHost = ""
	m_dbUser = ""
	m_dbPassword = ""
	m_db = ""

	# Table that contains the information about the payments
	# @var string
	m_dbPaymentsTable = ""

	# Table where you store character's names or accounts
	# @var string
	m_dbCharactersTable = ""

	# Secret key for your project, you can ask for it your account manager
	# @var string
	m_SecretKey = ""

	# List of allowed IP
	# @var List<string>
	m_AllowedIPs = []


# Class that implements "Virtual Currency Protocol"
#
# In this sample class you can find implementation of main methods of "Virtual Currency Protocol".
# To start script, replace parameters by your own parameters (based on your system).
#
# @version 1.0
# @author Xsolla
class VirtualCurrencyProtocolImplementation:
	m_Config = None
	m_Connect = None

	# Result codes
	SUCCESS							= 0
	TEMPORARY_ERROR					= 1
	CANCEL_NOT_FOUND				= 2
	INCORRECT_SIGNATURE				= 3
	INCORRECT_REQUEST_FORMAT		= 4
	OTHER_ERROR						= 5
	PAYMENT_CANNOT_BE_PROCESSED		= 6

	# Instantiates the VirtualCurrencyProtocolImplementation class
	def __init__(self, Config, strRemoteAddress, strCommand, strId, strSum, strV1, strV2, strV3, strMD5):
		self.m_Config = Config
		self.m_Connect = None

		try:
			self.m_Connect = MySQLdb.connect(Config.m_dbHost, Config.m_dbUser, Config.m_dbPassword, Config.m_db)
			self.ProcessRequest(strRemoteAddress, strCommand, strId, strSum, strV1, strV2, strV3, strMD5)
		except:
			self.ErrorCheckResponse("Unable to connect to database: " + str(sys.exc_info()[1]))
		finally:
			if(self.m_Connect):
				self.m_Connect.close()

	# Checks signature
	# @param string strSecretKey
	# @param string strMD5
	# @return bool
	def CheckSignature(self, strSecretKey, strMD5):
		strResult = md5.new(strSecretKey).digest()

		if(strMD5.lower() != strResult.lower()):
			return(False)

		return(True)

	# Method for nickname check
	# @param string strV1
	# @param string strV2
	# @param string strV3
	# @param string strMD5
	def ProcessCheckRequest(self, strV1, strV2, strV3, strMD5):
		try:
			iCode = self.OTHER_ERROR
			strDescription = ""

			# Checking existance of v1 - nickname, account and so on
			if(not strV1):
				raise Exception("User ID is undefined")

			# Checking existance of v2, v3. If you don't support these parametres, please comment out these lines
			if(not strV2):
				raise Exception("User ID is undefined")

			if(not strV3):
				raise Exception("User ID is undefined")

			# Checking existance of md5 param
			if(not strMD5):
				raise Exception("Signature is undefined")

			# Checking signature
			if(self.CheckSignature("check" + strV1 + self.m_Config.m_SecretKey, strMD5) == False):
				raise Exception("Incorrect signature")

			iCode, strDescription = self.Check(strV1, strV2, strV3)

			# Generating response
			self.GenerateCheckResponse(iCode, strDescription)
		except:
			self.ErrorCheckResponse(str(sys.exc_info()[1]))

	# Pay method
	# @param string strId
	# @param string strSum
	# @param string strV1
	# @param string strV2
	# @param string strV3
	# @param string strMD5
	def ProcessPayRequest(self, strId, strSum, strV1, strV2, strV3, strMD5):
		try:
			# Neccessary checking
			if(not strId):
				raise Exception("Invoice is undefined")

			if(not strV1):
				raise Exception("User ID is undefined")

			# If you don't support these parameters, please comment out these lines
			if(not strV2):
				raise Exception("User ID is undefined")

			if(not strV3):
				raise Exception("User ID is undefined")

			if(not strSum):
				raise Exception("Amount is undefined")

			if(not strMD5):
				raise Exception("Signature is undefined")

			if(self.CheckSignature("pay" + strV1 + strId + self.m_Config.m_SecretKey, strMD5) == False):
				raise Exception("Incorrect signature")

			iCode, strDescription, strIdShop = self.Pay(strId, strSum, strV1, strV2, strV3)
			self.GeneratePayResponse(iCode, strDescription, strId, strIdShop, strSum)
		except:
			self.ErrorPayResponse(str(sys.exc_info()[1]))

	# Payment cancel method
	# @param string strId
	# @param string strMD5
	def ProcessCancelRequest(self, strId, strMD5):
		try:
			if(not strId):
				raise Exception("Invoice is undefined")

			if(not strMD5):
				raise Exception("Signature is undefined")

			if(self.CheckSignature("cancel" + strId + self.m_Config.m_SecretKey, strMD5) == False):
				raise Exception("Incorrect signature")

			iCode, strDescription = self.Cancel(strId)

			# Generate response
			self.GenerateCancelResponse(iCode, strDescription)
		except:
			self.ErrorCancelResponse(str(sys.exc_info()[1]))

	# Main method
	# @param string strRemoteAddress
	# @param string strCommand
	# @param string strId
	# @param string strSum
	# @param string strV1
	# @param string strV2
	# @param string strV3
	# @param string strMD5
	def ProcessRequest(self, strRemoteAddress, strCommand, strId, strSum, strV1, strV2, strV3, strMD5):
		try:
			# Checking the IP-address
			if(not strRemoteAddress in self.m_Config.m_AllowedIPs):
				raise Exception("IP address is not allowed")

			if(not strCommand):
				raise Exception("Command is undefined")

			if(strCommand == "check"):
				self.ProcessCheckRequest(strV1, strV2, strV3, strMD5)
			elif(strCommand == "pay"):
				self.ProcessPayRequest(strId, strSum, strV1, strV2, strV3, strMD5)
			elif(strCommand == "cancel"):
				self.ProcessCancelRequest(strId, strMD5)
			else:
				raise Exception("Incorrect command")
		except:
			self.ErrorCheckResponse(str(sys.exc_info()[1]))

	# Generating response when using nickname check method
	# @param int iCode
	# @param string strDescription
	def GenerateCheckResponse(self, iCode, strDescription):
		strXML = "<?xml version=\"1.0\"?>"
		strXML += "<response>"
		strXML += "<result>" + str(iCode) + "</result>"
		strXML += "<comment>" + strDescription + "</comment>"
		strXML += "</response>"

		print "Content-Type", "text/xml charset=utf-8\r\n\r\n"
		print strXML

	# Generating response when using pay method
	# @param int iCode
	# @param string strDescription
	# @param string strInvoice
	# @param string strOrder
	# @param string strSum
	def GeneratePayResponse(self, iCode, strDescription, strInvoice, strOrder, strSum):
		strXML = "<?xml version=\"1.0\"?>"
		strXML += "<response>"
		strXML += "<id>" + strInvoice + "</id>"
		strXML += "<id_shop>" + strOrder + "</id_shop>"
		strXML += "<sum>" + strSum + "</sum>"
		strXML += "<result>" + str(iCode) + "</result>"
		strXML += "<comment>" + strDescription + "</comment>"
		strXML += "</response>"

		print "Content-Type", "text/xml charset=utf-8\r\n\r\n"
		print strXML

	# Generating response when using payment cancel method
	# @param int iCode
	# @param string strDescription
	def GenerateCancelResponse(self, iCode, strDescription):
		strXML = "<?xml version=\"1.0\"?>"
		strXML += "<response>"
		strXML += "<result>" + str(iCode) + "</result>"
		strXML += "<comment>" + strDescription + "</comment>"
		strXML += "</response>"

		print "Content-Type", "text/xml charset=utf-8\r\n\r\n"
		print strXML

	def ErrorCheckResponse(self, strMessage):
		self.GenerateCheckResponse(self.PAYMENT_CANNOT_BE_PROCESSED, strMessage)

	def ErrorPayResponse(self, strMessage):
		self.GeneratePayResponse(self.PAYMENT_CANNOT_BE_PROCESSED, strMessage, "", "", "")

	def ErrorCancelResponse(self, strMessage):
		self.GenerateCancelResponse(self.PAYMENT_CANNOT_BE_PROCESSED, strMessage)

	#####################################
	### THIS SECTION IS THE ONLY PLACE WHERE YOU SHOULD WRITE CODE ####
	#####################################

	# YOUR CHECK CODE HERE
	def Check(self, strV1, strV2, strV3):
		# Code example

		# If you don't use v2, v3, use the below
		#strCommand = "SELECT count(1) as cnt FROM " + self.m_Config.m_dbCharactersTable + " WHERE v1 = %s"
		strCommand = "SELECT count(1) as cnt FROM " + self.m_Config.m_dbCharactersTable + " WHERE v1 = %s AND v2 = %s AND v3 = %s"
		# performing query
		oCursor = self.m_Connect.cursor()
		oCursor.execute(strCommand, (strV1, strV2, strV3,))
		oRow = oCursor.fetchone()
		# if nickname exists
		if(oRow[0] > 0):
			iCode = self.SUCCESS
			strDescription = "OK"
		else:
			iCode = self.PAYMENT_CANNOT_BE_PROCESSED
			strDescription = "Character doesn't exist."
		oCursor.close()
		return(iCode, strDescription)

	# YOUR PAY CODE HERE
	def Pay(self, strId, strSum, strV1, strV2, strV3):
		# Code example

		# Looking for payment with such id
		strCommand = "SELECT `id` FROM " + self.m_Config.m_dbPaymentsTable + " WHERE `invoice` = %s"
		# performing query
		oCursor = self.m_Connect.cursor()
		oCursor.execute(strCommand, (strId,))
		# getting result
		# If there is no payment with this id, insert it
		if(oCursor.rowcount <= 0):
			# If you don't use v2, v3, use the below
			#strSQL = string("INSERT INTO `") + self.m_Config.m_dbPaymentsTable + string("` (`v1`, `amount`, `invoice`, `date_add`, `canceled`) VALUES (") + strV1 + string(", ") + strSum + string(", ") + strId + string(", NOW(), \"0\")")
			#strCommand = "INSERT INTO `" + self.m_Config.m_dbPaymentsTable + "` (`v1`, `amount`, `invoice`, `date_add`, `canceled`) VALUES (%s, %s, %s, NOW(), \"0\")"
			strCommand = "INSERT INTO `" + self.m_Config.m_dbPaymentsTable + "` (`v1`, `v2`, `v3`, `amount`, `invoice`, `date_add`, `canceled`) VALUES (%s, %s, %s, %s, %s, NOW(), \"0\")"
			# performing query
			oCursor.execute(strCommand, (strV1, strV2, strV3, strSum, strId,))
			# if insert successful
			if(oCursor.rowcount > 0):
				strCommand = "SELECT LAST_INSERT_ID()"
				oCursor.execute(strCommand)
				oRow = oCursor.fetchone()
				strIdShop = str(oRow[0])
				iCode = self.SUCCESS
				strDescription = "OK"
			else:
				strIdShop = ""
				iCode = self.OTHER_ERROR
				strDescription = "Couldn't insert data. See mysql errors"
		else:
			# If payment with such xsolla id exists, successful
			iCode = self.SUCCESS
			strDescription = "OK"
			oRow = oCursor.fetchone()
			strIdShop = str(oRow[0])
		self.m_Connect.commit()
		oCursor.close()
		return(iCode, strDescription, strIdShop)

	# YOUR CANCEL CODE HERE
	def Cancel(self, strId):
		# Code example

		# Canceling payment
		strCommand = "UPDATE `" + self.m_Config.m_dbPaymentsTable + "` SET `canceled` = \"1\", `date_cancel` = NOW() WHERE `invoice` = %s"
		# performing query
		oCursor = self.m_Connect.cursor()
		oCursor.execute(strCommand, (strId,))
		if(oCursor.rowcount > 0):
			iCode = self.SUCCESS
			strDescription = "OK"
		else:
			iCode = self.CANCEL_NOT_FOUND
			strDescription = "Payment with given ID does not exist"
		self.m_Connect.commit()
		oCursor.close()
		return(iCode, strDescription)
