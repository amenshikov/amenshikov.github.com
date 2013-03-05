<%
' Virtual Currency Protocol Implementation. ASP/VB Sample.
'
' @version 1.0
' @author Xsolla


' Config structure
Class XsollaConfig
	' Parameters for database connection
	' @var string
	Public m_dbHost
	Public m_dbUser
	Public m_dbPassword
	Public m_db

	' Table that contains the information about the payments
	' @var string
	Public m_dbPaymentsTable

	' Table where you store character's names or accounts
	' @var string
	Public m_dbCharactersTable

	' Secret key for your project, you can ask for it your account manager
	' @var string
	Public m_SecretKey

	' List of allowed IP
	' @var array
	Public m_AllowedIPs
End Class

' Result codes
Const RESULTCODE_SUCCESS						= 0
Const RESULTCODE_TEMPORARY_ERROR				= 1
Const RESULTCODE_CANCEL_NOT_FOUND				= 2
Const RESULTCODE_INCORRECT_SIGNATURE			= 3
Const RESULTCODE_INCORRECT_REQUEST_FORMAT		= 4
Const RESULTCODE_OTHER_ERROR					= 5
Const RESULTCODE_PAYMENT_CANNOT_BE_PROCESSED	= 6

Class VirtualCurrencyProtocolImplementation
	' Functions that implement "Virtual Currency Protocol"
	'
	' In this sample you can find implementation of main methods of "Virtual Currency Protocol".
	' To start, replace parameters by your own parameters (based on your system).
	'
	' @version 1.0
	' @author Xsolla

	' Checks signature
	' @param string strSecretKey
	' @param string strMD5
	' @return Boolean
	Private Function CheckSignature(strSecretKey, strMD5)
		Dim strResult

		strResult = MD5(strSecretKey)

		If (strMD5 <> strResult) Then
			CheckSignature = False
		Else
			CheckSignature = True
		End If
	End Function

	' Method for nickname check
	' @param string strV1
	' @param string strV2
	' @param string strV3
	' @param string strMD5
	Private Sub ProcessCheckRequest(Config, pConnect, strV1, strV2, strV3, strMD5)
		If (strV1 = "") Then ' Checking existance of v1 - nickname, account and so on
			ErrorCheckResponse("User ID is undefined")
		ElseIf (strV2 = "") Then ' Checking existance of v2, v3. If you don't support these parametres, please comment out these lines
			ErrorCheckResponse("User ID is undefined")
		ElseIf (strV3 = "") Then
			ErrorCheckResponse("User ID is undefined")
		ElseIf (strMD5 = "") Then ' Checking existance of md5 param
			ErrorCheckResponse("Signature is undefined")
		Else ' Checking signature
			Dim strSignature

			strSignature = "check" & strV1 & Config.m_SecretKey
			If (not CheckSignature(strSignature, strMD5)) Then
				ErrorCheckResponse("Incorrect signature")
			Else
				Dim iCode
				Dim strDescription

				iCode = Check(Config, pConnect, strV1, strV2, strV3, strDescription)

				' Generating response
				GenerateCheckResponse iCode, strDescription
			End If
		End If
	End Sub

	' Pay method
	' @param string strId
	' @param string strSum
	' @param string strV1
	' @param string strV2
	' @param string strV3
	' @param string strMD5
	Private Sub ProcessPayRequest(Config, pConnect, strId, strSum, strV1, strV2, strV3, strMD5)
		' Neccessary checking
		If (strId = "") Then
			ErrorPayResponse("Invoice is undefined")
		ElseIf (strV1 = "") Then
			ErrorPayResponse("User ID is undefined")
		ElseIf (strV2 = "") Then ' If you don't support these parameters, please comment out these lines
			ErrorPayResponse("User ID is undefined")
		ElseIf (strV3 = "") Then
			ErrorPayResponse("User ID is undefined")
		ElseIf (strSum = "") Then
			ErrorPayResponse("Amount is undefined")
		ElseIf (strMD5 = "") Then
			ErrorPayResponse("Signature is undefined")
		Else
			Dim strSignature

			strSignature = "pay" & strV1 & strId & Config.m_SecretKey
			If (not CheckSignature(strSignature, strMD5)) Then
				ErrorPayResponse("Incorrect signature")
			Else
				Dim iCode
				Dim strDescription
				Dim strIdShop

				iCode = Pay(Config, pConnect, strId, strSum, strV1, strV2, strV3, strDescription, strIdShop)
				GeneratePayResponse iCode, strDescription, strId, strIdShop, strSum
			End If
		End If
	End Sub

	' Payment cancel method
	' @param string strId
	' @param string strMD5
	Private Sub ProcessCancelRequest(Config, pConnect, strId, strMD5)
		If (strId = "") Then
			ErrorCancelResponse("Invoice is undefined")
		ElseIf (strMD5 = "") Then
			ErrorCancelResponse("Signature is undefined")
		Else
			Dim strSignature

			strSignature = "cancel" & strId & Config.m_SecretKey
			If (not CheckSignature(strSignature, strMD5)) Then
				ErrorCancelResponse("Incorrect signature")
			Else
				Dim iCode
				Dim strDescription

				iCode = Cancel(Config, pConnect, strId, strDescription)

				' Generate response
				GenerateCancelResponse iCode, strDescription
			End If
		End If
	End Sub

	' Main method
	' @param XsollaConfig Config
	' @param string strRemoteAddress
	' @param string strCommand
	' @param string strId
	' @param string strSum
	' @param string strV1
	' @param string strV2
	' @param string strV3
	' @param string strMD5
	Public Sub ProcessRequest(Config, strRemoteAddress, strCommand, strId, strSum, strV1, strV2, strV3, strMD5)
		Dim pConnect
		Dim bRemoteAddressAllowed

		Set pConnect = Server.CreateObject("ADODB.Connection")
		pConnect.Open("Driver={MySQL ODBC 5.2w Driver};Server=" & Config.m_dbHost & ";Database=" & Config.m_db & ";User=" & Config.m_dbUser & ";Password=" & Config.m_dbPassword & ";Option=3;")

		' Checking the IP-address
		bRemoteAddressAllowed = False
		For Each strAllowedIP In Config.m_AllowedIPs
			If (strRemoteAddress = strAllowedIP) Then
				bRemoteAddressAllowed = True
				Exit For
			End If
		Next
		If (bRemoteAddressAllowed = False) Then
			ErrorCheckResponse("IP address is not allowed")
		Else
			If (strCommand = "") Then
				ErrorCheckResponse("Command is undefined")
			Else
				If (strCommand = "check") Then
					ProcessCheckRequest Config, pConnect, strV1, strV2, strV3, strMD5
				ElseIf (strCommand = "pay") Then
					ProcessPayRequest Config, pConnect, strId, strSum, strV1, strV2, strV3, strMD5
				ElseIf (strCommand = "cancel") Then
					ProcessCancelRequest Config, pConnect, strId, strMD5
				Else
					ErrorCheckResponse("Incorrect command")
				End If
			End If
		End If

		pConnect.Close()
	End Sub

	' Generating response when using nickname check method
	' @param int iCode
	' @param string strDescription
	Private Sub GenerateCheckResponse(iCode, strDescription)
		Response.Write("Content-Type: text/xml charset=utf-8" & vbCrLf & vbCrLf)

		Response.Write("<?xml version=""1.0""?>")
		Response.Write("<response>")
		Response.Write("<result>" & CStr(iCode) & "</result>")
		Response.Write("<comment>" & strDescription & "</comment>")
		Response.Write("</response>")
	End Sub

	' Generating response when using pay method
	' @param int iCode
	' @param string strDescription
	' @param string strInvoice
	' @param string strOrder
	' @param string strSum
	Private Sub GeneratePayResponse(iCode, strDescription, strInvoice, strOrder, strSum)
		Response.Write("Content-Type: text/xml charset=utf-8" & vbCrLf & vbCrLf)

		Response.Write("<?xml version=""1.0""?>")
		Response.Write("<response>")
		Response.Write("<id>" & strInvoice & "</id>")
		Response.Write("<id_shop>" & strOrder & "</id_shop>")
		Response.Write("<sum>" & strSum & "</sum>")
		Response.Write("<result>" & CStr(iCode) & "</result>")
		Response.Write("<comment>" & strDescription & "</comment>")
		Response.Write("</response>")
	End Sub

	' Generating response when using payment cancel method
	' @param int iCode
	' @param string strDescription
	Private Sub GenerateCancelResponse(iCode, strDescription)
		Response.Write("Content-Type: text/xml charset=utf-8" & vbCrLf & vbCrLf)

		Response.Write("<?xml version=""1.0""?>")
		Response.Write("<response>")
		Response.Write("<result>" & CStr(iCode) & "</result>")
		Response.Write("<comment>" & strDescription & "</comment>")
		Response.Write("</response>")
	End Sub

	Private Sub ErrorCheckResponse(strMessage)
		GenerateCheckResponse RESULTCODE_PAYMENT_CANNOT_BE_PROCESSED, strMessage
	End Sub

	Private Sub ErrorPayResponse(strMessage)
		GeneratePayResponse RESULTCODE_PAYMENT_CANNOT_BE_PROCESSED, strMessage, "", "", ""
	End Sub

	Private Sub ErrorCancelResponse(strMessage)
		GenerateCancelResponse RESULTCODE_PAYMENT_CANNOT_BE_PROCESSED, strMessage
	End Sub

	' THIS SECTION IS THE ONLY PLACE WHERE YOU SHOULD WRITE CODE

	' YOUR CHECK CODE HERE
	Private Function Check(Config, pConnect, strV1, strV2, strV3, strDescription)
		Dim strSQL
		Dim oCommand
		Dim oResult
		Dim iCode

		' Code example

		' If you don't use v2, v3, use the string below
		'strSQL = "SELECT count(1) as cnt FROM " & Config.m_dbCharactersTable &" WHERE v1 = ?"
		strSQL = "SELECT count(1) as cnt FROM " & Config.m_dbCharactersTable & " WHERE v1 = ? AND v2 = ? AND v3 = ?"
		Set oCommand = Server.CreateObject("ADODB.Command")
		oCommand.ActiveConnection = pConnect
		oCommand.CommandType = 1 'adCmdText
		oCommand.CommandText = strSQL
		oCommand.Parameters.Append(oCommand.CreateParameter(, 200, 1, Len(strV1), strV1)) ' adVarChar, adParamInput
		oCommand.Parameters.Append(oCommand.CreateParameter(, 200, 1, Len(strV2), strV2)) ' adVarChar, adParamInput
		oCommand.Parameters.Append(oCommand.CreateParameter(, 200, 1, Len(strV3), strV3)) ' adVarChar, adParamInput

		' performing query
		Set oResult = oCommand.Execute()
		If (oResult.EOF) Then
			strDescription = "Failed to query database"
			Check = RESULTCODE_OTHER_ERROR
		End If

		' getting result
		' if nickname exists
		If (CLng(oResult.Fields(0)) > 0) Then
			iCode = RESULTCODE_SUCCESS
			strDescription = "OK"
		Else
			iCode = RESULTCODE_PAYMENT_CANNOT_BE_PROCESSED
			strDescription = "Character doesn't exist."
		End If

		Check = iCode
	End Function

	' YOUR PAY CODE HERE
	Private Function Pay(Config, pConnect, strId, strSum, strV1, strV2, strV3, strDescription, strIdShop)
		Dim strSQL
		Dim oCommand
		Dim oResult
		Dim iCode

		' Code example

		' Looking for payment with such id
		strSQL = "SELECT `id` FROM " & Config.m_dbPaymentsTable & " WHERE `invoice` = ?"
		Set oCommand = Server.CreateObject("ADODB.Command")
		oCommand.ActiveConnection = pConnect
		oCommand.CommandType = 1 'adCmdText
		oCommand.CommandText = strSQL
		oCommand.Parameters.Append(oCommand.CreateParameter(, 200, 1, Len(strId), strId)) ' adVarChar, adParamInput
		' performing query
		Set oResult = oCommand.Execute()
		' getting result
		' If there is no payment with this id, insert it
		If (oResult.EOF) Then
			Dim iIdShop

			' If you don't use v2, v3, use the string below
			'strSQL = "INSERT INTO `" & Config.m_dbPaymentsTable & "` (`v1`, `amount`, `invoice`, `date_add`, `canceled`) VALUES (?, ?, ?, ?, ?, NOW(), '0')"
			strSQL = "INSERT INTO `" & Config.m_dbPaymentsTable & "` (`v1`, `v2`, `v3`, `amount`, `invoice`, `date_add`, `canceled`) VALUES (?, ?, ?, ?, ?, NOW(), '0')"
			Set oCommand = Server.CreateObject("ADODB.Command")
			oCommand.ActiveConnection = pConnect
			oCommand.CommandType = 1 'adCmdText
			oCommand.CommandText = strSQL
			oCommand.Parameters.Append(oCommand.CreateParameter(, 200, 1, Len(strV1), strV1)) ' adVarChar, adParamInput
			oCommand.Parameters.Append(oCommand.CreateParameter(, 200, 1, Len(strV2), strV2)) ' adVarChar, adParamInput
			oCommand.Parameters.Append(oCommand.CreateParameter(, 200, 1, Len(strV3), strV3)) ' adVarChar, adParamInput
			oCommand.Parameters.Append(oCommand.CreateParameter(, 200, 1, Len(strSum), strSum)) ' adVarChar, adParamInput
			oCommand.Parameters.Append(oCommand.CreateParameter(, 200, 1, Len(strId), strId)) ' adVarChar, adParamInput
			' performing query
			Set oResult = oCommand.Execute()
			' if insert successful
			strSQL = "SELECT LAST_INSERT_ID() FROM `" & Config.m_dbPaymentsTable
			Set oCommand = New ADODB.Command
			oCommand.ActiveConnection = pConnect
			oCommand.CommandType = 1 'adCmdText
			oCommand.CommandText = strSQL
			Set oResult = oCommand.Execute()
			iIdShop = CLng(oResult.Fields(0))
			If (iIdShop <> 0) Then
				iCode = RESULTCODE_SUCCESS
				strDescription = "OK"
				strIdShop = iIdShop
			Else
				iCode = RESULTCODE_OTHER_ERROR
				strDescription = "Couldn't insert data. See mysql errors"
				strIdShop = ""
			End If
		Else
			' If payment with such xsolla id exists, successful
			iCode = RESULTCODE_SUCCESS
			strDescription = "OK"
			strIdShop = CStr(oResult.Fields(0))
		End If

		Pay = iCode
	End Function

	' YOUR CANCEL CODE HERE
	Private Function Cancel(Config, pConnect, strId, strDescription)
		Dim strSQL
		Dim oCommand
		Dim iRecordsAffected
		Dim iCode

		' Code example

		' Canceling payment
		strSQL = "UPDATE `" & Config.m_dbPaymentsTable & "` SET `canceled` = '1', `date_cancel` = NOW() WHERE `invoice` = ?"
		Set oCommand = Server.CreateObject("ADODB.Command")
		oCommand.ActiveConnection = pConnect
		oCommand.CommandType = 1 'adCmdText
		oCommand.CommandText = strSQL
		oCommand.Parameters.Append(oCommand.CreateParameter(, 200, 1, Len(strId), strId))
		' performing query
		iRecordsAffected = 0
		oCommand.Execute(iRecordsAffected)
		If (iRecordsAffected > 0) Then
			iCode = RESULTCODE_SUCCESS
			strDescription = "OK"
		Else
			iCode = RESULTCODE_CANCEL_NOT_FOUND
			strDescription = "Payment with given ID does not exist"
		End If

		Cancel = iCode
	End Function
End Class
%>