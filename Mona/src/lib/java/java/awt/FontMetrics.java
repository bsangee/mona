/*
Copyright (c) 2005 bayside

Permission is hereby granted, free of charge, to any person 
obtaining a copy of this software and associated documentation files 
(the "Software"), to deal in the Software without restriction, 
including without limitation the rights to use, copy, modify, merge, 
publish, distribute, sublicense, and/or sell copies of the Software, 
and to permit persons to whom the Software is furnished to do so, 
subject to the following conditions:

The above copyright notice and this permission notice shall be 
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF 
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY 
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, 
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE 
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

package java.awt;

/**
 �t�H���g���N���X
 */
public class FontMetrics {
	/** �t�H���g�I�t�Z�b�g���X�g���� */
	public static int offsetListLength;
	/** �t�H���g�I�t�Z�b�g���X�g */
	public static int[] offsetList;
	/** �t�H���g�f�[�^ */
	public static byte[] defaultFontData;
	/** �t�H���g�X�^�C�� */
	private int fontStyle;

	private native void create(byte[] data);

	/** �R���X�g���N�^ */
	public FontMetrics() {
		this.fontStyle = Font.PLAIN;
		if (defaultFontData == null) {
			this.defaultFontData = new byte[167921];
			create(this.defaultFontData);
		}
		
		// ���i�[�t�H���g�̍\��
		// 3 - 0 �t�H���g��
		// ? - 3 �t�H���g�\���� ( ? = �t�H���g�� )
		// 
		// �t�H���g�\����
		// 3 - 0 UCS-4 (UTF-16)
		//     4 �� (fw)
		//     5 ���� (fh)
		// ? - 6 �r�b�g�p�^�[�� ( ? = (fw * fh + 7) / 8 )
		
		offsetListLength = (defaultFontData[1] & 0xFF) << 8 | defaultFontData[0] & 0xFF;
		
		// �����܂ł̃I�t�Z�b�g(byte)
		offsetList = new int[65536];
		
		// �I�t�Z�b�g���X�g�����
		int pos = 4;
		for (int i = 0; i < offsetListLength; i++) {
			offsetList[(defaultFontData[pos + 1] & 0xFF) << 8 | defaultFontData[pos] & 0xFF] = pos;
			pos += 4;
			int fw = defaultFontData[pos++] & 0xFF;
			int fh = defaultFontData[pos++] & 0xFF;
			pos += (int)((fw * fh + 7) / 8);
		}
	}
	
	/**
	 �����̕��𓾂�
	 @param ch ����
	 */
	public int getCharWidth(char ch) {
		// null�`�F�b�N
		if (this.defaultFontData == null) return 0;
		
		if ((this.fontStyle & 0x100) == Font.FIXED) {
			if (ch < 128 || 0xff60 < ch) {
				return 6;
			} else {
				return 12;
			}
		} else {
			return defaultFontData[offsetList[ch] + 4];
		}
	}
	
	/**
	 ������̕��𓾂�
	 @param str ������i�����s�Ή��j
	 */
	public int getWidth(String str) {
		// null�`�F�b�N
		if (str.length() == 0 || this.defaultFontData == null) return 0;
		
		int w = 0;
		for (int i = 0; i < str.length(); i++) {
			char c = str.charAt(i);
			if (c == '\n') {
				break;
			}
			w += getCharWidth(c);
		}
		
		return w;
	}
	
	/**
	 ������̍����𓾂�
	 @param str ������i�����s�Ή��j
	 */
	public int getHeight(String str) {
		int h = 12;
		for (int i = 0; i < str.length(); i++) {
			if (str.charAt(i) == '\n') {
				h += 12; // ���i�[�t�H���g�͍�����12�h�b�g
			}
		}
		
		return h;
	}
	
	/** �t�H���g�X�^�C���𓾂� */
	public int getFontStyle() { return this.fontStyle; }
	
	/** �t�H���g�X�^�C����ݒ肷�� */
	public void setFontStyle(int style) { this.fontStyle = style; }
}