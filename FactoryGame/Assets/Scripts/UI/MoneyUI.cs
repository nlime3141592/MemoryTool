using UnityEngine;
using UnityEngine.Pool;

namespace nl
{
    public class MoneyUI : MonoBehaviour
    {
        public int value = 0;
        public float dist = 60.0f;
        public float time = 1.0f;

        private ObjectPool<DeltaMoneyUI> _pool;

        private void Awake()
        {
            _pool = new ObjectPool<DeltaMoneyUI>(
                OnCreateDeltaMoneyUI,
                null,
                null,
                null);
        }

        void Start()
        {

        }

        private void Update()
        {
            if (Input.GetKeyDown(KeyCode.F5))
            {
                DeltaMoneyUI ui = _pool.Get();
                ui.gameObject.SetActive(true);
                ui.SetDeltaMoney(value);
                ui.Distance = dist;
                ui.Lifetime = time;
            }
        }

        private DeltaMoneyUI OnCreateDeltaMoneyUI()
        {
            Transform newChild = GameObject.Instantiate(transform.GetChild(0));

            newChild.SetParent(this.transform, false);

            DeltaMoneyUI ui = newChild.GetComponent<DeltaMoneyUI>();
            ui.MyPool = _pool;

            return ui;
        }
    }
}