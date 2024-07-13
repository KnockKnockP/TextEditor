WebApplicationBuilder webApplicationBuilder = WebApplication.CreateBuilder();
WebApplication webApplication = webApplicationBuilder.Build();

static IResult Zip()
{
    using MemoryStream memoryStream = new();
    new HttpClient().GetStreamAsync("https://github.com/KnockKnockP/TextEditor/archive/refs/heads/master.zip").Result.CopyTo(memoryStream);

    return Results.File(memoryStream.ToArray(), "application/zip");
}

webApplication.MapGet("/", () => Zip());
webApplication.MapGet("/\r", () => Zip());

webApplication.Run("http://192.168.219.101");