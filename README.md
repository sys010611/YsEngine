# ゲームエンジン開発プロジェクト（OpenGL / C++）

## 基本情報

* **ジャンル**：ゲームエンジン
* **開発人数**：1人
* **開発環境**：C++、OpenGL、GitHub

---
<img width="804" height="468" alt="image" src="https://github.com/user-attachments/assets/c87e5cf9-5dab-4297-91cc-b06245d1d193" />
<img width="804" height="452" alt="image" src="https://github.com/user-attachments/assets/034ab16a-5c0b-4259-98fc-bf9bef485678" />
<img width="821" height="462" alt="image" src="https://github.com/user-attachments/assets/03698b87-ef2e-48cd-9211-240793288055" />
<img width="821" height="462" alt="image" src="https://github.com/user-attachments/assets/d26b944c-8d71-4a79-ab46-6c7369a2be7f" />

---

## 実装機能

* Skeletal Animation
* Scene Hierarchy / Inspector
* Terrain Collision
* Skybox
* Gizmo

---

## 使用技術

* Normal Mapping
* Tessellation Shader

---

## プロジェクト概要

OpenGL を活用したゲームエンジン開発プロジェクトです。
商用ゲームエンジンに近い GUI および操作体系を再現しており、
プレイモードではキャラクターを直接操作して移動することができます。

また、Phong Reflection Model を利用したライティングや、
Tessellation を使用した地形レンダリングも実装しました。

---

## 設計について

すべてのゲームオブジェクトは **Entity** という最上位クラスを継承して管理されます。
その中でも Light や Camera などの主要要素は共通機能を抽出し、
抽象クラスとして設計することで、オブジェクト指向を意識した構造になっています。

---

