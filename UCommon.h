#ifndef UCOMMON_H
#define UCOMMON_H

// �g���@�\���g�p����Ă��܂� : �E�Ӓl�̃N���X�����Ӓl�Ɏg�p����܂��B���̃��[�j���O��}������B
#pragma warning(disable: 4238)

// XMMatrixTranspose()�ŃG���[�ɂȂ�̂�SIMD���߃Z�b�g�g��Ȃ��悤�ɂ���
#define _XM_NO_INTRINSICS_

#include <windows.h>
#include <tchar.h>
#include <memory>
#include <mmsystem.h>  // timeGetTime()

#pragma comment( lib, "winmm.lib" )   // timeGetTime()

// ���������[�N�������Ƀf�o�b�K�ɏo�͂�����e���킩��₷������
#if defined(DEBUG) || defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC                            // malloc�ɂ�郁�������[�N���o��CPP�t�@�C�����ƍs���o�͎w��
#define NEW  new(_NORMAL_BLOCK, __FILE__, __LINE__)  // new �ɂ�郁�������[�N���o��CPP�t�@�C�����ƍs���o�͎w��
#else
#define NEW  new
#endif

// ���������
#define SAFE_RELEASE(x) if( x != nullptr ){ x->Release(); x = nullptr; }
#define SAFE_DELETE(x)  if( x != nullptr ){ delete x;  x = nullptr; }
#define SAFE_DELETE_ARRAY(x)  if( x != nullptr ){ delete[] x;  x = nullptr; }
#define SAFE_FREE(x)  if( x != nullptr ){ free( x );  x = nullptr; }

#endif
