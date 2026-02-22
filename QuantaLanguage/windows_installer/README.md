# How to Make the Windows Installer

Follow these simple steps to turn your Quanta code into a professional Windows Installer `.exe` file!

### Step 1: Get Your Windows `quanta.exe`
Since you are on a Mac, you need GitHub to build the Windows version of Quanta for you.

1. Commit and push your latest code to GitHub.
2. Go to your GitHub repository -> Click on the **Actions** tab at the top.
3. Click on the latest "Windows Build" run.
4. Scroll to the bottom to the **Artifacts** section and download the `quanta-windows-exe` zip file.
5. Extract the zip file. Inside, you will find `quanta.exe`.

### Step 2: Download Inno Setup (On a Windows PC)
To create the final installer, you (or a friend with a Windows PC) will need to use a free tool called Inno Setup.

1. Go to a Windows PC.
2. Download Inno Setup from here: https://jrsoftware.org/isdl.php (Download the "ispack-setup.exe" or "innosetup-q.q.q.exe").
3. Install Inno Setup on the Windows PC.

### Step 3: Put Files in the Right Place
On that Windows PC, you need two files in the exact same folder structure as your code:

1. Copy the `windows_installer` folder from your Mac to the Windows PC.
2. Inside that folder is `quanta.iss`.
3. Create a folder named `build` right next to the `windows_installer` folder.
4. Put the `quanta.exe` (that you downloaded from GitHub in Step 1) inside that `build` folder.

It should look exactly like this:
```text
SomeFolder/
├── build/
│   └── quanta.exe
└── windows_installer/
    └── quanta.iss
```

### Step 4: Create the Final Installer
1. Double-click on the `quanta.iss` file. It will open in the Inno Setup program.
2. At the top of the Inno Setup window, there is a green "Play" button (it says **Compile**). Click it!
3. Wait a few seconds for it to finish.
4. Go to the `windows_installer` folder. You will see a new folder named `Output`.
5. Open the `Output` folder. Inside is your final, professional installer:
   **`Quanta_Installer_v1.0.exe`**

### Step 5: Test It!
Double click `Quanta_Installer_v1.0.exe`! It will open a beautiful setup wizard. Once you click through it, Quanta will be permanently installed on the Windows PC, and you can open Command Prompt anywhere and type `quanta` to run it!
