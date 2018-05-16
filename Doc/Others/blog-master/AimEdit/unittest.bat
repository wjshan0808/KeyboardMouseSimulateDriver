:restart
@echo off

set timeout=3
@echo "Try to Aim edit control, in %timeout% seconds"
ping -n %timeout% -w 1000 1.0.0.1 > NUL
aimedit "95599#个人网上银行用户名登录#AfxWnd80su"
::aimedit "boc#中国银行#ATL:07765390"
::aimedit "icbc#中国工商银行新一代网上银行#ATL:Edit"
::aimedit "95559#交通银行个人网银#ATL:Edit"
::aimedit "cmbchina#pbsz.ebank.cmbchina.com#ATL:07741200"

@echo "Try to input password, in %timeout% seconds"
::ping -n %timeout% -w 1000 1.0.0.1 > NUL
keyinput "abcAB123!@"

@echo "Input is Done"

goto restart
