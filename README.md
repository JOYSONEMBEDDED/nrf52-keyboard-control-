# USB HID Generic Example - nRF52840 DK

This project demonstrates the usage of the **USBD HID Generic** example on the **nRF52840 DK (PCA10056)** using **nRF5 SDK v17.1.0** and **Segger Embedded Studio**. The output of the HID device can be monitored using **PuTTY**.

---

## 📁 Project Setup

1. **Download SDK:**

   Download the **nRF5 SDK v17.1.0 (ddde560)** from the official Nordic website:  
   [https://www.nordicsemi.com/Products/Development-software/nRF5-SDK](https://www.nordicsemi.com/Products/Development-software/nRF5-SDK)

2. **Extract SDK:**

   Extract the SDK to:
C:\nrf52sdk\nRF5_SDK_17.1.0_ddde560\

markdown
Copy
Edit

3. **Project Location:**

Copy the example project from:
C:\nrf52sdk\nRF5_SDK_17.1.0_ddde560\examples\peripheral\usbd_hid_generic

css
Copy
Edit
To:
C:\nrf52sdk\nRF5_SDK_17.1.0_ddde560\examples\MYPROJECTS\usbd_hid_generic\

markdown
Copy
Edit

4. **Open Project:**

Open the project file:
C:\nrf52sdk\nRF5_SDK_17.1.0_ddde560\examples\MYPROJECTS\usbd_hid_generic\pca10056\blank\ses\usbd_hid_generic_pca10056.emProject

yaml
Copy
Edit
using **Segger Embedded Studio (SES)**.

---

## 🔌 Hardware Setup

- **Board:** nRF52840 DK (PCA10056)
- **USB Connections:**
- Use the **main J-Link USB** (top side) to:
 - Flash the firmware
 - Monitor output via UART (PuTTY)
- Use the **second USB port near the RESET button** for **USB HID functionality**

---

## 🛠️ Building and Flashing

1. Make sure SES is configured for the correct SDK path.
2. Press **Build** and **Download** in SES to flash the firmware.
3. Connect the board as described above.

---

## 🖥️ Monitoring Output

1. Open **PuTTY**.
2. Choose **Serial** mode.
3. Select the COM port assigned to the board’s virtual COM (check Device Manager).
4. Set **baud rate** to `115200`.
5. You should see the HID device logs or responses here.

---

## 📌 Notes

- Do **not change** the default SDK folder name or structure.
- Ensure all paths are correct; SES uses relative paths to the SDK.
- The project will **not build properly** if copied outside the SDK or if SDK path is renamed.

---

## ✅ Tested Environment

- nRF5 SDK v17.1.0 (ddde560)
- Segger Embedded Studio v5.68+
- nRF52840 DK (PCA10056)
- Windows 10 / 11
- PuTTY v0.78+

---

## 📂 File Structure

C:\nrf52sdk\nRF5_SDK_17.1.0_ddde560
└── examples
└── MYPROJECTS
└── usbd_hid_generic
└── pca10056
└── blank
└── ses
└── usbd_hid_generic_pca10056.emProject

yaml
Copy
Edit

---

## 📧 Support

If you face build or USB connection issues, make sure:
- The J-Link drivers are installed
- The board is powered properly
- The SDK path matches exactly

---
