Option Explicit
   

Type jabDTO
  inputstring As String
  filename As String
  color_number As Long
  symbol_number As Long
  module_size As Long
  master_symbol_width As Long
  master_symbol_height As Long
  symbol_positions() As Long 'Liste
  symbol_positions_number As Long
  symbol_versions_x As Long
  symbol_versions_y As Long
  symbol_versions_number As Long
  symbol_ecc_levels() As Long 'Liste
  symbol_ecc_levels_number As Long
  color_space As Long
End Type

Type jab_bitmap_mod
    width As Long
    height As Long
    bits_per_pixel As Long
    bits_per_channel As Long
    channel_count As Long
    pixel() As Byte
End Type

Declare Function writeToTmpFile _
        Lib "c:\Users\fabio\Desktop\jabcode-master\dll_test\jabcodeWrapper\jabwrapper.dll" ( _
              ptrFoo As jabDTO _
        ) As LongPtr
        
Declare Function writeToByteArray _
        Lib "c:\Users\fabio\Desktop\jabcode-master\dll_test\jabcodeWrapper\jabwrapper.dll" ( _
              ptrFoo As jabDTO, ptrBitmap As jab_bitmap_mod _
        ) As LongPtr


Declare PtrSafe Function SetCurrentDirectoryA _
Lib "kernel32" (ByVal lpPathName As String) As Long

Private Const CP_UTF8 As Long = 65001
Declare Function MultiByteToWideChar Lib "kernel32" ( _
  ByVal CodePage As Long, _
  ByVal dwFlags As Long, _
  ByVal lpMultiByteStr As LongPtr, _
  ByVal cbMultiByte As Long, _
  ByVal lpWideCharStr As LongPtr, _
  ByVal cchWideChar As Long _
) As Long

' noetig fuer String Rueckgabe -> Konversion

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

' ACHTUNG: Nach jedem ausführen auf STOP klicken, weil sonst gibt es eine Zugriffsverletzung und WORD crasht..

Sub main() ' {

    'Workaround wenn DLLs nicht registriert worden sind
    SetCurrentDirectoryA ("C:\Users\fabio\Desktop\jabcode-master\dll_test\jabcodeWrapper\dlls") ' AENDERN IN LOKALEN DLL PFAD

    Dim x As jabDTO
    x.inputstring = "hello world"
    x.filename = "C:\Users\fabio\Desktop\jabcode-master\dll_test\jabcodeWrapper\test.png" ' WENN AUSKOMMENTIERT ODER NULL, DANN WIRD IN %TEMP%/tmpjabcode.png GESPEICHERT
    'x.color_number = 4
   
    Dim s As String
    s = utf8PtrToString(writeToTmpFile(x))
    MsgBox "s = " & s
    
    ' Fuehrt zum Word Crash -> Immer wenn auf uebergebenen Typ geschrieben wird und dieser zurueckgegeben wird.. (alternativen muessen noch getestet werden)
    'Dim bitmap As jab_bitmap_mod
    'Dim test As String
    'test = writeToByteArray(x, bitmap)
    'MsgBox test

End Sub ' }



