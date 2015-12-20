Function Transform([string]$mode, [string]$src, [string]$dst)
{
  $pinfo = New-Object System.Diagnostics.ProcessStartInfo
  $pinfo.FileName = "des.exe"
  $pinfo.UseShellExecute = $false
  $pinfo.Arguments = "$mode -f 0x1337 $src $dst"
  $des = New-Object System.Diagnostics.Process
  $des.StartInfo = $pinfo
  $des.Start() | Out-Null
  $des.WaitForExit()
  return $des.ExitCode
}

Function RunTests
{
  $files = @(
    "0b.txt",
    "6b.txt",
    "17b.txt",
    "1337b.txt",
    "4096b.txt",
    "4098b.txt"
  )  
  foreach ($plainText in $files)
  {
    $cipherText = $plainText+".des"
    $decipherText = $cipherText+".out"
    $ret = Transform "encrypt" $plainText $cipherText
    if ($ret -ne 0)
    {
      Write "encrypt $plainText : internal error occured ($ret)."
      return
    }
    $ret = Transform "decrypt" $cipherText $decipherText
    if ($ret -ne 0)
    {
      Write "decrypt $cipherText : internal error occured ($ret)."
      return
    }
    $plainTextHash = (Get-FileHash $plainText).hash;
    $decipherTextHash = (Get-FileHash $decipherText).hash;
    if ($plainTextHash -ne $decipherTextHash)
    {
      Write "$plainText : plaintext/deciphertext hashes differ."
      return
    }
    Remove-Item $cipherText
    Remove-Item $decipherText
  }
  Write "all tests passed."
  return
}

RunTests
