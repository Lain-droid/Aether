# Project: "Aether" - Advanced Luau Scripting Environment (User-Mode)

**Version:** 1.0.0 (Architecture and Usage Guide)
**Platform:** Windows 10/11 (x64/x86)
**Target:** Roblox (User-Mode, Educational)

---

**DISCLAIMER:** This project is for education and research only. Do not use for cheating or violating platform rules. You are solely responsible for your use.

---

## 1. Overview

"Aether" is a modern, user-mode Luau scripting environment featuring a VSCode-like editor (monochrome theme), real-time console, and a guided setup flow. Kernel components have been removed.

Projenin temel amacı, geliştiricilere ve araştırmacılara, Roblox'un çalışma zamanı (runtime) ortamıyla güvenli ve kontrollü bir şekilde etkileşim kurma olanağı sağlamaktır.

## 2. Architecture

Two layers: **User-Mode Backend (C++)** and **Frontend (C# WPF)**, communicating over a secure IPC (Named Pipes). No kernel drivers.

### 2.1. Diagram

```
[ FRONTEND (C# WPF) ]
 |    - VSCode-like editor (sidebar, center editor, bottom terminal)
 |    - Setup flow (risk acceptance, inject, Go to Roblox)
 |    - Settings (editor font/autocomplete, AI sensitivity)
 |
 <--- [ Secure IPC (Named Pipes) ] --->
 |
[ BACKEND (C++) ]
 |    - IPC server (Inject op=1, Execute op=2, Config op=3, Bypass op=4)
 |    - Luau VM compile/execute
 |    - AI controller (risk + sensitivity)
```

### 2.2. Flow

1.  Startup & Inject:
    - Start the app; Setup window appears.
    - Accept risks; click Inject. Frontend sends inject and starts bypass (op=1, op=4) via IPC.
    - Click "Go to Roblox" to open the Roblox client.

2.  Script Execution:
    - Write Luau script in the editor; click Execute.
    - Script is sent over IPC (op=2) and executed by the VM.

3.  Terminal:
    - Realtime messages appear in the bottom terminal. Filters/search can be added.

## 3. Setup & Usage

### 3.1. Requirements
- Windows 10/11 (x64)
- Admin privileges recommended

### 3.2. Steps
1. Launch `Aether.exe` (as Administrator recommended)
2. In Setup, accept risks and click Inject
3. Click "Go to Roblox" to open the client
4. Use the editor (left Explorer, center editor, bottom terminal)

### 3.3. Cleanup
Cleanup işlemi, sistemin kararlılığını korumak için kritik öneme sahiptir. Aşağıdaki durumlarda otomatik olarak tetiklenir:
- Injection işlemi başarısız olduğunda.
- Kullanıcı uygulamayı kapattığında.
- Roblox prosesi beklenmedik bir şekilde sonlandığında.

**Cleanup Adımları:**
1. Release all user-mode resources
2. Stop IPC
3. Clear state

## 4. Features

### 4.1. Editor
- **Dil Desteği:** Tam Luau dil desteği.
- **Özellikler:**
    - **Syntax Highlighting:** Kodun okunabilirliğini artırmak için anahtar kelimeler, değişkenler ve fonksiyonlar renklendirilir.
    - **Autocomplete:** Roblox API'si ve genel Luau fonksiyonları için otomatik tamamlama önerileri sunar. `game:GetService("...")` gibi yaygın kalıpları tanır.
    - **Hata Gösterimi:** Yazım sırasında anlık olarak syntax hatalarını kırmızı alt çizgilerle belirtir.
    - **Kod Katlama:** Uzun kod bloklarını (`function`, `if`, `for` döngüleri) katlayarak daha temiz bir görünüm sağlar.
    - **Temalar:** Açık (Light) ve Koyu (Dark) tema seçenekleri mevcuttur.

### 4.2. Terminal
- **Gerçek Zamanlı Çıktı:** Roblox'ta çalışan script'lerinizin `print()`, `warn()` gibi komutlarla ürettiği tüm çıktılar bu panelde anlık olarak görünür.
- **Filtreleme:** "Error", "Warning", "Info" gibi seviyelere göre logları filtreleyebilirsiniz.
- **Arama:** Konsol çıktısı içinde metin araması yapabilirsiniz.
- **Kopyalama:** Tek bir satırı veya tüm konsol içeriğini panoya kopyalayabilirsiniz.

## 5. Security & Error Handling

### 5.1. Güvenlik Katmanları
- **User-Mode Sınırları:** Kernel bileşenleri kaldırıldığı için risk yüzeyi daraltıldı.
- **Signature Masking (User-Mode):** Yalnızca user-mode bileşenlerde geçerli olacak şekilde sadeleştirildi.
- **Anti-Debug/Tamper:** Uygulama süreçlerinde temel korumalar etkin.
- **Güvenli IPC:** Frontend ve backend arasındaki tüm iletişim şifrelenir ve doğrulanır.

### 5.2. Hata Yönetimi
- **Net Bilgilendirme:** Hatalar, teknik detayları içeren ancak kullanıcıyı paniğe sevk etmeyen net mesajlarla sunulur. Örneğin: "Injection Failed: Could not obtain a handle to the target process (Error Code: 5)."
- **Otomatik Kurtarma:** Injection hatalarında sistem otomatik olarak `Cleanup` yapar ve kullanıcıya yeniden deneme seçeneği sunar.
- **Logging:** Tüm kritik işlemler (injection, cleanup, hatalar) `AetherVisor.log` adlı bir dosyaya kaydedilir. Bu dosya, sorun giderme için kullanılabilir.
