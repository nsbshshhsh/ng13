import os
import subprocess
import sys

def create_shortcut():
    try:
        desktop = os.path.join(os.environ['USERPROFILE'], 'Desktop')
        shortcut_path = os.path.join(desktop, 'CineMax Booking.lnk')
        current_dir = os.path.dirname(os.path.abspath(__file__))
        target_path = os.path.join(current_dir, 'Khoi_Dong_He_Thong.bat')
        
        # PowerShell script to create shortcut, using double-byte safe character representation
        ps_cmd = f"""
        $s = (New-Object -ComObject WScript.Shell).CreateShortcut([System.Text.Encoding]::UTF8.GetString([System.Convert]::FromBase64String('{base64_encode(shortcut_path)}')));
        $s.TargetPath = [System.Text.Encoding]::UTF8.GetString([System.Convert]::FromBase64String('{base64_encode(target_path)}'));
        $s.WorkingDirectory = [System.Text.Encoding]::UTF8.GetString([System.Convert]::FromBase64String('{base64_encode(current_dir)}'));
        $s.IconLocation = 'shell32.dll,279';
        $s.Save();
        """
        
        subprocess.run(["powershell", "-NoProfile", "-Command", ps_cmd], capture_output=True, check=True)
        print("Shortcut created successfully!")
    except Exception as e:
        print(f"Error creating shortcut: {e}", file=sys.stderr)

def base64_encode(s):
    import base64
    return base64.b64encode(s.encode('utf-8')).decode('utf-8')

if __name__ == '__main__':
    create_shortcut()
