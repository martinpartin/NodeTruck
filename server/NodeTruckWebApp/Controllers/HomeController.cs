using System.Diagnostics;
using Microsoft.AspNetCore.Mvc;
using Microsoft.Extensions.Options;
using NodeTruckWebApp.Models;

namespace NodeTruckWebApp.Controllers;
public class HomeController : Controller
{
    private readonly IMqttService _mqttService;
    private readonly IOptions<MqttSettings> _settings;
    private readonly ILogger<HomeController> _logger;

    public HomeController(IMqttService mqttService, IOptions<MqttSettings> settings, ILogger<HomeController> logger)
    {
        _mqttService = mqttService;
        _settings = settings;
        _logger = logger;
    }

    public IActionResult Index()
    {
        return View();
    }

    [HttpPost]
    public async Task<IActionResult> SendCommand(string command)
    {

        await _mqttService.PublishAsync(_settings.Value.ControlTopic, command);
        return RedirectToAction("Index");
    }

    [ResponseCache(Duration = 0, Location = ResponseCacheLocation.None, NoStore = true)]
    public IActionResult Error()
    {
        return View(new ErrorViewModel { RequestId = Activity.Current?.Id ?? HttpContext.TraceIdentifier });
    }
}
