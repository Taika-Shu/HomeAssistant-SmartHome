import React, { useState, useEffect } from 'react';
import axios from 'axios';

function App() {
  const [data, setData] = useState({
    status: "loading",
    indoor: { temp: 0, humidity: 0 },
    outdoor: { temp: 0, humidity: 0 },
    security_mode: 1,
    alarm_active: false,
    version: "1.0.0"
  });
  
  const [lastUpdate, setLastUpdate] = useState("刚刚");
  const [notification, setNotification] = useState({ show: false, message: "", type: "info" });

  useEffect(() => {
    const fetchData = async () => {
      try {
        const response = await axios.get('/api/status');
        setData(response.data);
        updateLastUpdateTime();
      } catch (err) {
        console.error('Error fetching data:', err);
        showNotification('无法连接到后端服务器', 'warning');
      }
    };

    // 每5秒更新一次数据
    const interval = setInterval(fetchData, 5000);
    fetchData(); // 立即获取一次数据

    return () => clearInterval(interval);
  }, []);

  const updateLastUpdateTime = () => {
    const now = new Date();
    const timeString = now.toLocaleTimeString('zh-CN', { 
      hour: '2-digit', 
      minute: '2-digit',
      second: '2-digit'
    });
    setLastUpdate(timeString);
  };

  const showNotification = (message, type) => {
    setNotification({ show: true, message, type });
    setTimeout(() => {
      setNotification(prev => ({ ...prev, show: false }));
    }, 3000);
  };

  const sendCommand = async (command) => {
    try {
      await axios.post('/api/command', { command });
      showNotification(`命令 ${command} 已发送`, 'success');
      // 立即获取最新数据
      const response = await axios.get('/api/status');
      setData(response.data);
      updateLastUpdateTime();
    } catch (err) {
      console.error('Error sending command:', err);
      showNotification('发送命令失败', 'warning');
    }
  };

  const getStatusColor = () => {
    if (data.status === "running") return "#4caf50";
    return "#f44336";
  };

  const isOnline = data.status === "running";

  return (
    <div style={{ 
      fontFamily: "'Segoe UI', 'Microsoft YaHei', sans-serif",
      backgroundColor: "#121212",
      color: "#e0e0e0",
      minHeight: "100vh",
      display: "flex",
      justifyContent: "center",
      alignItems: "center",
      padding: "20px"
    }}>
      <div style={{
        width: "100%",
        maxWidth: "800px",
        background: "linear-gradient(145deg, #1e1e1e, #2a2a2a)",
        borderRadius: "20px",
        boxShadow: "0 10px 30px rgba(0, 0, 0, 0.5)",
        overflow: "hidden",
        border: "1px solid #333"
      }}>
        <div style={{
          background: "linear-gradient(90deg, #0d7377, #14ffec)",
          padding: "25px 30px",
          textAlign: "center",
          position: "relative"
        }}>
          <h1 style={{
            fontSize: "2.2rem",
            fontWeight: "700",
            color: "white",
            textShadow: "0 2px 5px rgba(0, 0, 0, 0.3)",
            letterSpacing: "0.5px",
            margin: 0
          }}>
            <i className="fas fa-home" style={{ marginRight: "10px" }}></i>
            HomeAssistant 智能家居仪表板
          </h1>
          <div style={{
            position: "absolute",
            top: "25px",
            right: "30px",
            background: "rgba(0, 0, 0, 0.2)",
            padding: "8px 15px",
            borderRadius: "20px",
            fontWeight: "600",
            fontSize: "0.9rem",
            display: "flex",
            alignItems: "center",
            gap: "8px"
          }}>
            <div style={{
              width: "12px",
              height: "12px",
              borderRadius: "50%",
              backgroundColor: isOnline ? "#4caf50" : "#f44336",
              boxShadow: isOnline ? "0 0 8px #4caf50" : "0 0 8px #f44336"
            }}></div>
            <span>{isOnline ? "在线" : "离线"}</span>
          </div>
        </div>
        
        <div style={{ padding: "30px" }}>
          <div style={{
            display: "grid",
            gridTemplateColumns: "repeat(auto-fit, minmax(200px, 1fr))",
            gap: "25px",
            marginBottom: "40px"
          }}>
            <div style={{
              background: "rgba(40, 40, 40, 0.7)",
              borderRadius: "15px",
              padding: "25px",
              textAlign: "center",
              transition: "transform 0.3s, box-shadow 0.3s",
              border: "1px solid #333"
            }}>
              <div style={{
                fontSize: "2.5rem",
                marginBottom: "15px",
                color: "#14ffec"
              }}>
                <i className="fas fa-thermometer-half"></i>
              </div>
              <div style={{
                fontSize: "1rem",
                color: "#aaa",
                marginBottom: "8px",
                textTransform: "uppercase",
                letterSpacing: "1px"
              }}>
                室内温度
              </div>
              <div style={{
                fontSize: "2.2rem",
                fontWeight: "700",
                color: "white"
              }}>
                {data.indoor?.temp ? data.indoor.temp.toFixed(1) : 0}<span style={{ fontSize: "1.2rem", color: "#aaa", marginLeft: "5px" }}>°C</span>
              </div>
            </div>
            
            <div style={{
              background: "rgba(40, 40, 40, 0.7)",
              borderRadius: "15px",
              padding: "25px",
              textAlign: "center",
              transition: "transform 0.3s, box-shadow 0.3s",
              border: "1px solid #333"
            }}>
              <div style={{
                fontSize: "2.5rem",
                marginBottom: "15px",
                color: "#14ffec"
              }}>
                <i className="fas fa-tint"></i>
              </div>
              <div style={{
                fontSize: "1rem",
                color: "#aaa",
                marginBottom: "8px",
                textTransform: "uppercase",
                letterSpacing: "1px"
              }}>
                室内湿度
              </div>
              <div style={{
                fontSize: "2.2rem",
                fontWeight: "700",
                color: "white"
              }}>
                {data.indoor?.humidity ? data.indoor.humidity.toFixed(1) : 0}<span style={{ fontSize: "1.2rem", color: "#aaa", marginLeft: "5px" }}>%</span>
              </div>
            </div>
            
            <div style={{
              background: "rgba(40, 40, 40, 0.7)",
              borderRadius: "15px",
              padding: "25px",
              textAlign: "center",
              transition: "transform 0.3s, box-shadow 0.3s",
              border: "1px solid #333"
            }}>
              <div style={{
                fontSize: "2.5rem",
                marginBottom: "15px",
                color: "#14ffec"
              }}>
                <i className="fas fa-shield-alt"></i>
              </div>
              <div style={{
                fontSize: "1rem",
                color: "#aaa",
                marginBottom: "8px",
                textTransform: "uppercase",
                letterSpacing: "1px"
              }}>
                安全模式
              </div>
              <div style={{
                fontSize: "2.2rem",
                fontWeight: "700",
                color: "white"
              }}>
                {data.security_mode}
              </div>
            </div>
          </div>
          
          <div style={{ 
            textAlign: "center", 
            marginBottom: "40px", 
            color: "#888", 
            fontSize: "1rem" 
          }}>
            <i className="fas fa-code-branch"></i> 版本: <span>{data.version}</span>
          </div>
          
          <div style={{ 
            display: "flex", 
            flexWrap: "wrap", 
            gap: "20px", 
            justifyContent: "center" 
          }}>
            <button 
              onClick={() => sendCommand('!1')}
              style={{
                flex: "1",
                minWidth: "200px",
                background: "linear-gradient(90deg, #0d7377, #14b8a6)",
                border: "none",
                borderRadius: "12px",
                padding: "20px",
                color: "white",
                fontSize: "1.1rem",
                fontWeight: "600",
                cursor: "pointer",
                transition: "all 0.3s",
                display: "flex",
                flexDirection: "column",
                alignItems: "center",
                gap: "12px",
                boxShadow: "0 5px 15px rgba(0, 0, 0, 0.2)"
              }}
            >
              <i className="fas fa-home" style={{ fontSize: "1.8rem" }}></i>
              <span>使用 Home Assistant 数据</span>
            </button>
            
            <button 
              onClick={() => sendCommand('!2')}
              style={{
                flex: "1",
                minWidth: "200px",
                background: "linear-gradient(90deg, #6a11cb, #2575fc)",
                border: "none",
                borderRadius: "12px",
                padding: "20px",
                color: "white",
                fontSize: "1.1rem",
                fontWeight: "600",
                cursor: "pointer",
                transition: "all 0.3s",
                display: "flex",
                flexDirection: "column",
                alignItems: "center",
                gap: "12px",
                boxShadow: "0 5px 15px rgba(0, 0, 0, 0.2)"
              }}
            >
              <i className="fas fa-cloud" style={{ fontSize: "1.8rem" }}></i>
              <span>使用虚拟数据</span>
            </button>
            
            <button 
              onClick={() => sendCommand('ok')}
              style={{
                flex: "1",
                minWidth: "200px",
                background: "linear-gradient(90deg, #ff416c, #ff4b2b)",
                border: "none",
                borderRadius: "12px",
                padding: "20px",
                color: "white",
                fontSize: "1.1rem",
                fontWeight: "600",
                cursor: "pointer",
                transition: "all 0.3s",
                display: "flex",
                flexDirection: "column",
                alignItems: "center",
                gap: "12px",
                boxShadow: "0 5px 15px rgba(0, 0, 0, 0.2)"
              }}
            >
              <i className="fas fa-bell" style={{ fontSize: "1.8rem" }}></i>
              <span>确认警报</span>
            </button>
          </div>
          
          <div style={{
            marginTop: "30px",
            textAlign: "center",
            fontSize: "0.9rem",
            color: "#777",
            padding: "15px",
            background: "rgba(30, 30, 30, 0.7)",
            borderRadius: "10px",
            border: "1px solid #333"
          }}>
            <i className="fas fa-plug"></i> API 端点: <code>8080/api/status</code> | 最后更新: <span>{lastUpdate}</span>
          </div>
        </div>
      </div>
      
      {notification.show && (
        <div style={{
          position: "fixed",
          top: "30px",
          right: "30px",
          background: "#333",
          color: "white",
          padding: "15px 25px",
          borderRadius: "10px",
          boxShadow: "0 5px 15px rgba(0, 0, 0, 0.4)",
          zIndex: 100,
          display: "flex",
          alignItems: "center",
          gap: "12px",
          transform: "translateX(150%)",
          transition: "transform 0.5s cubic-bezier(0.68, -0.55, 0.27, 1.55)",
          animation: "slideIn 0.5s forwards"
        }}>
          <i className={`fas ${notification.type === 'success' ? 'fa-check-circle' : notification.type === 'warning' ? 'fa-exclamation-triangle' : 'fa-info-circle'}`}></i>
          <div>{notification.message}</div>
        </div>
      )}
      
      <style>{`
        @keyframes slideIn {
          from { transform: translateX(150%); }
          to { transform: translateX(0); }
        }
        .fas {
          font-family: "Font Awesome 5 Free";
          font-weight: 900;
        }
      `}</style>
    </div>
  );
}

export default App;
