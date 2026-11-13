# SPINEWISE

Aplikasi edukasi 3D interaktif tentang anatomi tulang belakang dan skoliosis (proyek Grafika Komputer).
Standalone Windows, Unity 2022.3 LTS (C#), fully offline. Lihat PRD v1.1 (tab "SPINEWISE (Revisi)").

> Aplikasi edukasi, **bukan** alat diagnosis.

## Setup (setiap anggota)

1. Install **Unity Hub** + **Unity 2022.3.62f1** (versi harus sama untuk semua anggota;
   lihat `ProjectSettings/ProjectVersion.txt`). Modul yang perlu dicentang: *Windows Build Support (Mono)*
   (sudah default di Windows) dan *Visual Studio* / VS Code editor.
2. Install **Git LFS** sekali saja: `git lfs install`
3. Clone repo:
   ```
   git clone <url-repo>
   ```
4. Unity Hub → **Add → Add project from disk** → pilih folder hasil clone → buka.
   Pembukaan pertama akan agak lama karena Unity membuat folder `Library/`.
5. Buka `Assets/Scenes/MainMenu.unity`. Console harus bersih (tanpa error merah).

## Scene (`Assets/Scenes`, sudah terdaftar di Build Settings)

| # | Scene | Isi | Owner |
|---|-------|-----|-------|
| 0 | MainMenu | Navigasi, disclaimer, panel About & Credits | Dinda (UI & Integration) |
| 1 | SkeletonExplorer | Skeleton, orbit camera, highlight spine | Rafiq (Anatomy & Animation) |
| 2 | SpineLab | Panel input parameter, spine prosedural, comparison | Diny (Spine) + Dealova (Camera/Render) |
| 3 | PostureLab | Karakter animasi, skeleton overlay, penjelasan | Rafiq (Anatomy & Animation) |

## Struktur folder

```
Assets/
  Scenes/           4 scene utama
  Scripts/
    Core/           SpineParams, navigasi scene, data
    Spine/          SpineGenerator, SpineDeformer, SpineController
    CameraControl/  OrbitCamera, ViewPresets
    UI/             panel input, validasi, menu
  Prefabs/          bagikan pekerjaan lewat prefab, bukan edit scene orang lain
  Materials/
  Models/           skeleton, vertebra (catat sumber + lisensi untuk Credits)
  Animations/       animasi Mixamo
  Resources/Data/   teks edukasi JSON (offline)
  ThirdParty/       aset/plugin pihak ketiga
```

## Aturan kerja Git

- **Satu owner per scene.** Jangan edit scene milik orang lain; buat prefab di `Assets/Prefabs` lalu minta owner memasangnya.
  Merge conflict di file `.unity` sulit diselesaikan.
- Selalu commit file `.meta` bersama asetnya.
- Branch per fitur: `feature/<nama-fitur>`, merge ke `main` lewat Pull Request.
- `git pull` sebelum mulai kerja, commit kecil-kecil.
- Model/tekstur/audio otomatis masuk Git LFS (lihat `.gitattributes`). Hindari file > 100 MB;
  decimate model di Blender dulu.
