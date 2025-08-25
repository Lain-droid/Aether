# Proje: "AetherVisor" - Gelişmiş Roblox Executor Platformu

**Sürüm:** 1.0.0 (Tasarım ve Mimari Belgesi)
**Platform:** Windows 10/11 (x64/x86)
**Hedef:** Roblox (Hyperion/Byfron Anti-Cheat)

---

**UYARI:** Bu proje, yalnızca Roblox platformunda yazılım geliştirme ve tersine mühendislik alanında eğitim ve araştırma amacıyla tasarlanmıştır. Aracın kötüye kullanılması, hile veya oyun kurallarının ihlali gibi amaçlarla kullanılması kesinlikle yasaktır ve tüm yasal sorumluluk kullanıcıya aittir. Geliştiriciler, projenin yasa dışı kullanımından sorumlu tutulamaz.

---

## 1. Proje Özeti

AetherVisor, Windows platformları için geliştirilmiş, Roblox'un en güncel anti-cheat sistemleri olan Hyperion ve Byfron'u atlatmayı hedefleyen, yüksek performanslı ve modüler bir Luau script executor projesidir. Proje, %0 ban riski ve tam güvenlik hedefleriyle tasarlanmıştır. Kullanıcıya modern bir arayüz üzerinden gelişmiş bir script editörü ve canlı konsol takibi imkanı sunar.

Projenin temel amacı, geliştiricilere ve araştırmacılara, Roblox'un çalışma zamanı (runtime) ortamıyla güvenli ve kontrollü bir şekilde etkileşim kurma olanağı sağlamaktır.

## 2. Sistem Mimarisi ve Tasarım

AetherVisor, üç ana katmandan oluşan modüler bir mimariye sahiptir: **Kernel-Mode Driver**, **User-Mode Backend** ve **User-Interface Frontend**. Bu katmanlar arasındaki iletişim, güvenli ve yüksek performanslı IPC (Inter-Process Communication) mekanizmaları üzerinden sağlanır.

### 2.1. Mimarinin Şematik Gösterimi

```
[ KULLANICI ARAYÜZÜ (FRONTEND - C# WPF) ]
 |    - Gelişmiş Luau Script Editörü
 |    - Canlı Roblox Console Görüntüleyici
 |    - Injection ve Durum Kontrol Paneli
 |
 <--- [ Güvenli IPC Kanalı (Named Pipes / Shared Memory) ] --->
 |
[ USER-MODE BACKEND (C++) ]
 |    - Ana Kontrol Mantığı
 |    - IPC Sunucusu
 |    - Yükleyici (Loader) ve Enjektör (Injector)
 |    - Script Yürütme Motoru (Luau -> Roblox)
 |    - Anti-Debug ve Anti-Tamper Teknikleri
 |
 <--- [ IOCTL (I/O Control) Arayüzü ] --->
 |
[ KERNEL-MODE DRIVER (C++) ]
     - Hyperion/Byfron Bypass Modülü
     - Bellek Okuma/Yazma Primitifleri (R/W)
     - Proses ve Thread Gizleme
     - Sistem Gözetiminden Kaçınma (ETW, DbgPrint Engelleme)
     - Güvenli Cleanup Mekanizması
```

### 2.2. Modüller ve Veri Akışı

1.  **Başlatma ve Injection:**
    *   Kullanıcı, AetherVisor frontend uygulamasını çalıştırır.
    *   Frontend, backend prosesini (headless) başlatır.
    *   Backend, sistemde gerekli yetkilere sahip olup olmadığını kontrol eder ve kernel-mode sürücüyü (AetherVisor.sys) sisteme yükler.
    *   Sürücü, Hyperion'un kernel seviyesindeki korumalarını etkisiz hale getirmek için gerekli bypass'ları uygular.
    *   Backend, Roblox prosesini bulur ve sürücünün sağladığı primitifleri kullanarak user-mode DLL'ini (payload) Roblox'un bellek alanına güvenli bir şekilde enjekte eder.
    *   **Başarısızlık Durumu:** Eğer bu adımlardan herhangi biri başarısız olursa, `Cleanup` süreci tetiklenir. Sürücü sistemden kaldırılır, enjekte edilen tüm bileşenler bellekten silinir ve kullanıcıya detaylı bir hata raporu sunulur.

2.  **Script Yürütme:**
    *   Kullanıcı, frontend'deki Luau editörüne script'ini yazar.
    *   "Execute" butonuna tıklandığında, script metni güvenli IPC kanalı üzerinden backend'e gönderilir.
    *   Backend, bu script'i Roblox'a enjekte edilmiş olan DLL'e iletir.
    *   DLL içerisindeki yürütme motoru, Luau script'ini Roblox'un kendi Lua sanal makinesi içinde çalıştırır.

3.  **Console Çıktısı:**
    *   Roblox'a enjekte edilen DLL, oyunun konsol fonksiyonlarını (örneğin `print`, `warn`) hook'lar.
    *   Bu fonksiyonlardan gelen çıktılar (mesajlar, hatalar, uyarılar) yakalanır ve IPC kanalı üzerinden backend'e, oradan da anlık olarak frontend'deki canlı konsol paneline iletilir.

## 3. Kurulum ve Kullanım

### 3.1. Gerekli Sistem Yapılandırması
- Windows 10/11 (x64).
- Secure Boot'un kapalı olması (Test-signing modunun aktif edilmesi gerekebilir).
- Yönetici (Administrator) yetkileri.

### 3.2. Kurulum Adımları
1.  **AetherVisor'ı Başlatma:** `AetherVisor.exe` uygulamasını yönetici olarak çalıştırın.
2.  **Otomatik Kurulum:** Uygulama ilk açıldığında "Kurulum Kontrolü" ekranı görünür.
    *   Bu ekranda, backend kernel sürücüsünün (`AetherVisor.sys`) sisteme yüklenmesini başlatır.
    *   Sürücü başarıyla yüklendikten sonra, uygulama Roblox prosesini tespit etmeye çalışır.
3.  **Injection:** Roblox çalıştığında, "Inject" butonuna basın.
    *   Backend, user-mode DLL'i Roblox prosesine enjekte eder.
    *   Injection başarılı olursa, ana menü (Script Editörü ve Console) görünür hale gelir.
    *   Başarısız olursa, bir hata mesajı gösterilir ve `Cleanup` işlemi otomatik olarak çalışır.

### 3.3. Cleanup (Temizleme) Süreci
Cleanup işlemi, sistemin kararlılığını korumak için kritik öneme sahiptir. Aşağıdaki durumlarda otomatik olarak tetiklenir:
- Injection işlemi başarısız olduğunda.
- Kullanıcı uygulamayı kapattığında.
- Roblox prosesi beklenmedik bir şekilde sonlandığında.

**Cleanup Adımları:**
1.  Roblox'a enjekte edilen tüm DLL'ler ve bellek yamaları geri alınır.
2.  Kernel-mode sürücü, tüm bypass'ları etkisiz hale getirir ve sistemden güvenli bir şekilde kaldırılır (unload).
3.  Tüm IPC kanalları ve diğer kaynaklar serbest bırakılır.

## 4. Özelliklerin Kullanımı

### 4.1. Script Editörü
- **Dil Desteği:** Tam Luau dil desteği.
- **Özellikler:**
    - **Syntax Highlighting:** Kodun okunabilirliğini artırmak için anahtar kelimeler, değişkenler ve fonksiyonlar renklendirilir.
    - **Autocomplete:** Roblox API'si ve genel Luau fonksiyonları için otomatik tamamlama önerileri sunar. `game:GetService("...")` gibi yaygın kalıpları tanır.
    - **Hata Gösterimi:** Yazım sırasında anlık olarak syntax hatalarını kırmızı alt çizgilerle belirtir.
    - **Kod Katlama:** Uzun kod bloklarını (`function`, `if`, `for` döngüleri) katlayarak daha temiz bir görünüm sağlar.
    - **Temalar:** Açık (Light) ve Koyu (Dark) tema seçenekleri mevcuttur.

### 4.2. Canlı Console Paneli
- **Gerçek Zamanlı Çıktı:** Roblox'ta çalışan script'lerinizin `print()`, `warn()` gibi komutlarla ürettiği tüm çıktılar bu panelde anlık olarak görünür.
- **Filtreleme:** "Error", "Warning", "Info" gibi seviyelere göre logları filtreleyebilirsiniz.
- **Arama:** Konsol çıktısı içinde metin araması yapabilirsiniz.
- **Kopyalama:** Tek bir satırı veya tüm konsol içeriğini panoya kopyalayabilirsiniz.

## 5. Güvenlik ve Hata Yönetimi

### 5.1. Güvenlik Katmanları
- **Kernel-Mode Bypass:** Anti-cheat sistemlerinin en derin seviyedeki tespit mekanizmalarını atlatır.
- **Signature Masking:** Enjekte edilen DLL'lerin ve sürücünün dosya ve bellek imzaları sürekli olarak maskelenir.
- **Anti-Debug/Tamper:** AetherVisor'ın kendi proseslerinin ve Roblox'un debug edilmesini veya dışarıdan müdahale edilmesini engeller.
- **Güvenli IPC:** Frontend ve backend arasındaki tüm iletişim şifrelenir ve doğrulanır.

### 5.2. Hata Yönetimi
- **Net Bilgilendirme:** Hatalar, teknik detayları içeren ancak kullanıcıyı paniğe sevk etmeyen net mesajlarla sunulur. Örneğin: "Injection Failed: Could not obtain a handle to the target process (Error Code: 5)."
- **Otomatik Kurtarma:** Injection hatalarında sistem otomatik olarak `Cleanup` yapar ve kullanıcıya yeniden deneme seçeneği sunar.
- **Logging:** Tüm kritik işlemler (injection, cleanup, hatalar) `AetherVisor.log` adlı bir dosyaya kaydedilir. Bu dosya, sorun giderme için kullanılabilir.
