public interface IMqttService
{
    Task PublishAsync(string topic, string payload);
    public event Action<string, string>? OnMessageReceived; 
}


