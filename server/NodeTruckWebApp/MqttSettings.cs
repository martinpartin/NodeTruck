public class MqttSettings
{
    public required  string Broker { get; set; }
    public int Port { get; set; }
    public required string Username { get; set; }
    public required string Password { get; set; }
    public required string ControlTopic { get; set; }
    public required string HeartbeatTopic { get; set; }
    public bool UseTLS { get; set; }
}


