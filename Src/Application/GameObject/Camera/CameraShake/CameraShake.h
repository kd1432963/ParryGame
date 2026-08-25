//============================================================
// 1回のカメラシェイクに必要な調整値
//============================================================
struct CameraShakeSettings
{
	// 揺れ続ける合計時間。単位は秒
	float			durationSeconds		= 0.0f;

	// 1秒間に作るランダムな揺れ先の数。大きいほど細かく震える
	float			samplesPerSecond	= 30.0f;

	// カメラのローカル XYZ 方向へ動ける最大距離
	Math::Vector3	maxPositionOffset	= Math::Vector3::Zero;

	// XYZ 回転の最大角度。X = 上下、Y = 左右、Z = 画面の傾き。単位は度数法
	Math::Vector3	maxRotationDegrees	= Math::Vector3::Zero;
};

//============================================================
// 描画用カメラへ加える位置と回転
// 回転値は調整しやすいように度数法で保持する
//============================================================
struct CameraShakeOffset
{
	Math::Vector3	positionOffset			= Math::Vector3::Zero;
	Math::Vector3	rotationOffsetDegrees	= Math::Vector3::Zero;
};

//============================================================
// カメラの揺れ生成だけを担当するコンポーネント
//============================================================
class CameraShake
{
public:

	//===========================================================
	// 公開関数群
	//===========================================================
	// 設定値を使って新しい揺れを開始する
	void Start(const CameraShakeSettings& settings);

	// 揺れ時間とサンプル間の補間を進める
	void Update(float deltaTime);

	// 揺れを終了し、すべての一時データを初期化する
	void Stop();

	bool IsActive() const{ return m_remainingTime > 0.0f;}

	const CameraShakeOffset& GetOffset() const{ return m_offset;}

private:

	//===========================================================
	// 内部関数群
	//===========================================================
	// 次に補間するランダムな位置と回転を作る
	void CreateNextSample();

	//===========================================================
	// 状態値
	//===========================================================
	CameraShakeSettings	m_settings;
	CameraShakeOffset	m_offset;

	Math::Vector3		m_sampleFromPosition	= Math::Vector3::Zero;
	Math::Vector3		m_sampleToPosition		= Math::Vector3::Zero;
	Math::Vector3		m_sampleFromRotation	= Math::Vector3::Zero;
	Math::Vector3		m_sampleToRotation		= Math::Vector3::Zero;
	
	float				m_remainingTime			= 0.0f;
	float				m_sampleElapsed			= 0.0f;
};