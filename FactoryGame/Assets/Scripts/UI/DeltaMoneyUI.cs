using UnityEngine;
using UnityEngine.Pool;
using UnityEngine.UI;

namespace nl
{
    public class DeltaMoneyUI : MonoBehaviour
    {
        public ObjectPool<DeltaMoneyUI> MyPool { get; set; } = null;
        public float Distance { get; set; } = 60.0f;
        public float Lifetime { get; set; } = 1.0f;

        private RectTransform _rt;
        private Text _txt;

        private float _r;
        private float _g;
        private float _b;
        private float _a;
        
        public void SetDeltaMoney(int value)
        {
            if (value < 0)
            {
                _r = 1.0f;
                _g = 0.0f;
                _b = 0.0f;
                _txt.text = $"- {-value}$";
            }
            else if (value > 0)
            {
                _r = 1.0f;
                _g = 0.92f;
                _b = 0.016f;
                _txt.text = $"+ {value}$";
            }
            else
            {
                _r = 1.0f;
                _g = 1.0f;
                _b = 1.0f;
                _txt.text = $"+ 0$";
            }
        }

        private void Awake()
        {
            _rt = GetComponent<RectTransform>();
            _txt = GetComponent<Text>();
        }

        private void OnEnable()
        {
            float x = _rt.localPosition.x;
            float y = 0.0f;
            float z = _rt.localPosition.z;

            _rt.localPosition = new Vector3(x, y, z);
        }

        private void Update()
        {
            float x = _rt.localPosition.x;
            float y = _rt.localPosition.y - Distance * Time.deltaTime / Lifetime;
            float z = _rt.localPosition.z;

            float coef = -4.0f / (Distance * Distance);
            float absy = Mathf.Abs(y);
            _a = Mathf.Clamp01(coef * absy * (absy - Distance));

            _rt.localPosition = new Vector3(x, y, z);
            _txt.color = new Color(_r, _g, _b, _a);

            if (y <= -Distance)
            {
                MyPool.Release(this);
                this.gameObject.SetActive(false);
            }
        }
    }
}