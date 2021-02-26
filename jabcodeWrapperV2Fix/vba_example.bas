Option Explicit
   

Type jabDTO
  inputstring As String
  filename As String
  color_number As Long
  symbol_number As Long
  module_size As Long
  master_symbol_width As Long
  master_symbol_height As Long
  symbol_positions As String ' eigentlich Liste -> aber Stringparse leichter
  symbol_positions_number As Long
  symbol_versions As String ' eigentlich Liste -> aber Stringparse leichter
  symbol_versions_number As Long
  symbol_ecc_levels As String ' eigentlich Liste -> aber Stringparse leichter
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

Private Declare Function FreeLibrary Lib "kernel32" (ByVal hLibModule As Long) As Long
Private Declare Function LoadLibrary Lib "kernel32" Alias "LoadLibraryA" (ByVal lpLibFileName As String) As Long

Private Declare PtrSafe Function GetModuleHandle Lib "kernel32" Alias "GetModuleHandleA" (ByVal GetModuleHandle As String) As LongPtr


Private Declare Function writeToTmpFile _
        Lib "C:\Users\fabio\Desktop\jabcode-master\dll_test\jabcodeWrapper\jabwrapper.dll" ( _
              ptrFoo As jabDTO _
        ) As LongPtr
        
Private Declare Function writeToByteArray _
        Lib "C:\Users\fabio\Desktop\jabcode-master\dll_test\jabcodeWrapper\jabwrapper.dll" ( _
              ptrFoo As jabDTO, ptrBitmap As jab_bitmap_mod _
        ) As LongPtr
        
Private Declare Function readFromJabPNG _
        Lib "C:\Users\fabio\Desktop\jabcode-master\dll_test\jabcodeWrapper\jabwrapper.dll" ( _
              ptrFoo As jabDTO _
        ) As LongPtr

' hier haette man eigentlich ein Interger/Long zurueckgeben koennen, aber Type Konversion kann gelegentlich CRASH erzeugen, daher hab ich die Schnittstelle genauso wie bei den anderen Funktionen gelassen ._.
Private Declare Function getMasterSymbolEncodingCapacity _
        Lib "C:\Users\fabio\Desktop\jabcode-master\dll_test\jabcodeWrapper\jabwrapper.dll" ( _
              ptrFoo As jabDTO _
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

' ACHTUNG: Niemals die Ziel PNG von irgendeinem Programm geöffnet haben :D (Alt : Nach jedem ausführen auf STOP klicken, weil sonst gibt es eine Zugriffsverletzung und WORD crasht..)
'           -- Es kann noch zu gelegentlichen crashes kommen, wenn zu schnell hintereinander nach dem Entladen, die gleiche DLL-Funktion wiederverwendet wird.. also noch nicht ganz perfekt

Sub main() ' {

    'Workaround wenn DLLs nicht registriert worden sind
    SetCurrentDirectoryA ("C:\Users\fabio\Desktop\jabcode-master\dll_test\jabcodeWrapper\dlls") ' AENDERN IN LOKALEN DLL PFAD

    Dim x As jabDTO
    x.inputstring = "longer test string is super duper duper super loooooooooooooooooooooooooooooooooooooooong1234567890987654321234567876543212345676543212345676543211234567654321ooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo"
    x.filename = "C:\Users\fabio\Desktop\jabcode-master\dll_test\jabcodeWrapper\test2.png" ' WENN AUSKOMMENTIERT ODER NULL, DANN WIRD IN %TEMP%/tmpjabcode.png GESPEICHERT
    x.color_number = 4
    'x.module_size = 40
    
    ' Beispiel encoding wie jabcodeWrite.exe - Usage example
    x.symbol_number = 3
    x.symbol_positions = "0 3 2"
    x.symbol_versions = "3 2 4 2 3 2"
    x.symbol_ecc_levels = "3 3 3"
    
    ' DLL explizit dynamisch laden:
    Dim lb As Long
    lb = LoadLibrary("C:\Users\fabio\Desktop\jabcode-master\dll_test\jabcodeWrapper\jabwrapper.dll")

    ' NUR UNTERSCHIEDLICHE DLL FUNKTIONEN AUFRUFEN!!
    
    ' CAPACITY WITH JABDTO -> ENCODING (IN BIT)
    Dim c As String
    c = utf8PtrToString(getMasterSymbolEncodingCapacity(x))
    MsgBox "kapazitaet (?) = " & c
   
    ' READ JABCODE FILE
    Dim r As String
    r = utf8PtrToString(readFromJabPNG(x))
    MsgBox "decodierter text = " & r
    
    ' WRITE JABCODE FILE
    Dim s As String
    s = utf8PtrToString(writeToTmpFile(x))
    MsgBox "pfad = " & s
    
  
    
    ' DLL ENTLADEN -> Kann in unterfunktion ausgelagert werden und sollte immer aufgerufen, werden sobald eine DLL Funktion nochmal aufgerufen wird... ist blöd, aber verhindert CRASH
    Do Until lb = 0
        FreeLibrary lb
        If CBool(Err.LastDllError) Then
            MsgBox "dll error" & " " & Err.LastDllError
            Err.Clear
            Exit Do
        End If

        lb = 0 ' Reset lb needed for test on next line

        ' Check if the dll really has been released...
        lb = GetModuleHandle("C:\Users\fabio\Desktop\jabcode-master\dll_test\jabcodeWrapper\jabwrapper.dll")
    Loop
    
    ' hier kann wieder eine DLL Funktion von oben aufgerufen werden
    
           
    ' getMasterSymbolEncodingCapacity benutzt gleiche Datentypen von writeToTmpFile, also wichtig vorher DLL entladen, falls writeToTmpFile aufgerufen wurde
    'Dim c As String
    'c = utf8PtrToString(getMasterSymbolEncodingCapacity(x))
    'MsgBox "kapazitaet (?) = " & c
          

End Sub ' }



