Option Explicit

Type FOO
  str As String
  num As Long
End Type

'
' "Normal" operation: the c-struct / VBA-type FOO is passed
' as a pointer to the DLL:
'
Declare PtrSafe Function passPtrFOO _
        Lib "c:\Users\fabio\Desktop\jabcode-master\dll_test\pointer-to-struct\the.dll" _
        ( _
                 ptrFoo As FOO _
        ) As Long

Private Const CP_UTF8 As Long = 65001
Declare Function MultiByteToWideChar Lib "kernel32" ( _
  ByVal CodePage As Long, _
  ByVal dwFlags As Long, _
  ByVal lpMultiByteStr As LongPtr, _
  ByVal cbMultiByte As Long, _
  ByVal lpWideCharStr As LongPtr, _
  ByVal cchWideChar As Long _
) As Long

'#if Win64 then
Function utf8PtrToString(ByVal pUtf8string As LongPtr) As String ' {
' #Else
'   function utf8PtrToString(byVal pUtf8string as long   ) as string
' #End if
'
'
    Dim buf    As String
    Dim cSize  As Long
    Dim mbVal  As Long
    
    cSize = MultiByteToWideChar(CP_UTF8, 0, pUtf8string, -1, 0, 0)
  ' cSize includes the terminating null character
    If cSize <= 1 Then
       utf8PtrToString = ""
       Exit Function
    End If
    
    utf8PtrToString = String(cSize - 1, "*") ' and a termintating null char.
    mbVal = MultiByteToWideChar(CP_UTF8, 0, pUtf8string, -1, StrPtr(utf8PtrToString), cSize)

    If mbVal = 0 Then
       Err.Raise 1000, "Error", "MultiByteToWideChar failed"
    End If

End Function ' }
    
    

Sub main()

    Dim f As FOO

    f.str = "this is hello world"
    f.num = 42

    Dim s As String
    s = utf8PtrToString(passPtrFOO(f))
    'Dim x As Boolean
    'x = (passPtrFOO(f))
    MsgBox "s = " & s

  '
  ' Pass the struct as pointer.
  ' Works ok
  '
  '  If Not passPtrFOO(f) Then
  '  if not passPtrFOO( ptrFoo := f ) then
  '     MsgBox "Unexpectedly, passPtrFOO returned false"
  '  End If

  '  If Not passPtrFOOasAny(f) Then
  '     MsgBox "Unexpectedly, passPtrFOOasAny returned false"
  '  End If

  '
  ' Trying to pass a null pointer.
  ' It does not work if (or because) the parameter is declared with 'as any':
  '
  '  If Not passPtrFOOasAny(0) Then
  '     MsgBox "Unexpectely, passPtrFOOasAny returned false"
  '  End If

  '
  ' Pass the number 0 as a longPtr which will be
  ' interpreted as a null pointer in the DLL:
  '
  '  If passNullPtrFOO(0) Then
  '     MsgBox "Unexpectely, passNullPtrFOO returned true"
  '  End If

  '
  ' varPtr creates a pointer - BUT the BSTR f.str is
  ' not converted into a c string. Instead of »hello world", just
  ' an »h« is received.
  '
  '  If Not passNullPtrFOO(VarPtr(f)) Then
  '     MsgBox "Unexpectedly, passNullPtrFOO returned false"
  '  End If

End Sub




